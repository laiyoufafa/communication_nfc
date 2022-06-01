/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "nfc_service.h"

#include "common_event_handler.h"
#include "common_event_manager.h"
#include "loghelper.h"
#include "nfc_controller.h"
#include "nfc_sdk_common.h"
#include "nfc_watch_dog.h"
#include "nfcc_host.h"
#include "want.h"

namespace OHOS {
namespace NFC {
const std::u16string NFC_SERVICE_NAME = OHOS::to_utf16("ohos.nfc.service");
int NfcService::nciVersion_ = 0x02;

std::weak_ptr<TAG::TagDispatcher> NfcService::GetTagDispatcher()
{
    return tagDispatcher_;
}

void NfcService::OnTagDiscovered(std::shared_ptr<NCI::ITagHost> tagHost)
{
    InfoLog("NfcService::OnTagDiscovered");
    eventHandler_->SendEvent<NCI::ITagHost>(static_cast<uint32_t>(NfcCommonEvent::MSG_TAG_FOUND), tagHost);
}

bool NfcService::IsNfcTaskReady(std::future<int>& future) const
{
    if (future.valid()) {
        if (std::future_status::ready != future.wait_for(std::chrono::seconds(1))) {
            return false;
        }
    }
    return true;
}

void NfcService::ExecuteTask(KITS::NfcTask param, bool saveState)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (nfcState_ == KITS::STATE_TURNING_OFF || nfcState_ == KITS::STATE_TURNING_ON) {
        ErrorLog("Execute task %{public}d from bad state %{public}d", param, nfcState_);
        return;
    }

    // Check the current state
    if (param == KITS::TASK_TURN_ON && nfcState_ == KITS::STATE_ON) {
        DebugLog("NFC Turn On");
        return;
    }
    if (param == KITS::TASK_TURN_OFF && nfcState_ == KITS::STATE_OFF) {
        DebugLog("NFC Turn Off");
        return;
    }

    if (saveState) {
        SaveNfcOnSetting(param == KITS::TASK_TURN_ON);
    }

    std::promise<int> promise;
    if (rootTask_) {
        if (!IsNfcTaskReady(future_)) {
            // working
            return;
        }
        if (task_ && task_->joinable()) {
            task_->join();
        }
        future_ = promise.get_future();
        task_ = std::make_unique<std::thread>(&NfcService::NfcTaskThread, this, param, std::move(promise));
    } else {
        rootTask_ = std::make_unique<std::thread>(&NfcService::NfcTaskThread, this, param, std::move(promise));
    }
}

void NfcService::SaveNfcOnSetting(bool on)
{
}

void NfcService::NfcTaskThread(KITS::NfcTask params, std::promise<int> promise)
{
    DebugLog("Nfc task thread params %{public}d", params);
    switch (params) {
        case KITS::TASK_TURN_ON:
            DoTurnOn();
            break;
        case KITS::TASK_TURN_OFF:
            DoTurnOff();
            break;
        case KITS::TASK_INITIALIZE: {
            bool initialized = false;
            nfccHost_->FactoryReset();
            initialized = DoTurnOn();
            DebugLog("initialized = %{public}d", initialized);
            break;
        }
        default:
            break;
    }
    promise.set_value_at_thread_exit(0);
    return;
}

bool NfcService::DoTurnOn()
{
    DebugLog("Nfc do turn on: current state %{public}d", nfcState_);
    UpdateNfcState(KITS::STATE_TURNING_ON);

    NfcWatchDog nfcWatchDog("DoTurnOn", WAIT_MS_INIT, nfccHost_);
    nfcWatchDog.Run();
    // Routing WakeLock acquire
    if (!nfccHost_->Initialize()) {
        ErrorLog("Nfc do turn on err");
        UpdateNfcState(KITS::STATE_OFF);
        // Routing Wake Lock release
        nfcWatchDog.Cancel();
        return false;
    }
    // Routing Wake Lock release
    nfcWatchDog.Cancel();

    nciVersion_ = nfccHost_->GetNciVersion();
    DebugLog("Get nci version: ver %{public}d", nciVersion_);

    UpdateNfcState(KITS::STATE_ON);
    return true;
}

/**
 * Disable all NFC adapter functions.
 * Does not toggle preferences.
 */
bool NfcService::DoTurnOff()
{
    DebugLog("Nfc do turn off: current state %{public}d", nfcState_);
    UpdateNfcState(KITS::STATE_TURNING_OFF);

    /* Sometimes mNfccHost.deinitialize() hangs, use a watch-dog.
     * Implemented with a new thread (instead of a Handler or AsyncTask),
     * because the UI Thread and AsyncTask thread-pools can also get hung
     * when the NFC controller stops responding */
    NfcWatchDog nfcWatchDog("DoTurnOff", WAIT_MS_SET_ROUTE, nfccHost_);
    nfcWatchDog.Run();

    bool result = nfccHost_->Deinitialize();
    DebugLog("NfccHost deinitialize result %{public}d", result);

    nfcWatchDog.Cancel();
    UpdateNfcState(KITS::STATE_OFF);
    return result;
}

void NfcService::UpdateNfcState(int newState)
{
    DebugLog("Update nfc state: oldState %{public}d, newState %{public}d", nfcState_, newState);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (newState == nfcState_) {
            return;
        }
        nfcState_ = newState;
    }

    // noitfy the common event for nfc state changed.
    AAFwk::Want want;
    want.SetAction(KITS::COMMON_EVENT_NFC_ACTION_STATE_CHANGED);
    want.SetParam(KITS::NFC_EXTRA_STATE, newState);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    EventFwk::CommonEventManager::PublishCommonEvent(data);
}

int NfcService::GetNfcState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return nfcState_;
}

bool NfcService::IsNfcEnabled()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return (nfcState_ == KITS::STATE_ON);
}

void NfcService::HandleScreenChanged(int screenState)
{
    screenState_ = screenState;
    DebugLog("Screen changed screenState %{public}d", screenState_);
}

void NfcService::HandlePackageUpdated()
{
    DebugLog("HandlePackageUpdated, unimplimentation...");
}

std::weak_ptr<NfcService> NfcService::GetInstance() const
{
    return nfcService_;
}

bool NfcService::Initialize()
{
    nfcService_ = shared_from_this();
    InfoLog("Nfc service initialize.");
    if (nfccHost_) {
        nfccHost_->SetNfccHostListener(nfcService_);
    } else {
        nfccHost_ = std::make_shared<NFC::NCI::NfccHost>(nfcService_);
    }

    // inner message handler, used by other modules as initialization parameters
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("common event handler");
    eventHandler_ = std::make_shared<CommonEventHandler>(runner, shared_from_this());
    tagDispatcher_ = std::make_shared<TAG::TagDispatcher>(shared_from_this());

    // To be structured after Tag and HCE, the controller module is the controller of tag and HCE module
    nfcControllerImpl_ = new NfcControllerImpl(shared_from_this());

    eventHandler_->Intialize(tagDispatcher_);
    runner->Run();
    // NFC ROOT
    ExecuteTask(KITS::TASK_INITIALIZE);
    return true;
}

NfcService::NfcService(std::unique_ptr<NFC::NCI::INfccHost> nfccHost)
    : nfccHost_(std::move(nfccHost)),
    nfcControllerImpl_(nullptr),
    eventHandler_(nullptr),
    tagDispatcher_(nullptr),
    nfcState_(KITS::STATE_OFF)
{
}

NfcService::~NfcService()
{
    nfcControllerImpl_ = nullptr;
    if (task_ && task_->joinable()) {
        task_->join();
    }
    if (rootTask_ && rootTask_->joinable()) {
        rootTask_->join();
    }
}
}  // namespace NFC
}  // namespace OHOS