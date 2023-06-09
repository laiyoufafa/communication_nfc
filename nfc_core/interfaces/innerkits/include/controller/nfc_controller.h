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
#ifndef NFC_CONTROLLER_H
#define NFC_CONTROLLER_H

#include "nfc_controller_proxy.h"

namespace OHOS {
namespace NFC {
namespace KITS {
static const std::string NFC_SERVICE_NAME = "nfc";
const std::string NFC_EXTRA_STATE = "ohos.nfc.extra.ADAPTER_STATE";
const std::string COMMON_EVENT_NFC_ACTION_STATE_CHANGED = "usual.event.nfc.action.ADAPTER_STATE_CHANGED";

class NfcController final {
public:
    explicit NfcController();
    ~NfcController();

    /**
     * @Description Get an object of nfc controller.
     * @param void
     * @return an object of nfc controller
     */
    static NfcController &GetInstance();
    /**
     * @Description Turn on Nfc of the device.
     * @param void
     * @return Errorcode of turn on nfc. if return 0, means successful.
     */
    int TurnOn();
    /**
     * @Description Turn off Nfc of the device.
     * @param void
     * @return Errorcode of turn off nfc. if return 0, means successful.
     */
    int TurnOff();
    /**
     * @Description Get nfc state of device.
     * @param void
     * @return nfc state.
     */
    int GetNfcState();

private:

    static void InitNfcController();

private:
    static bool initialized_;

    static std::shared_ptr<NfcControllerProxy> nfcControllerProxy_;
    static std::weak_ptr<OHOS::NFC::INfcControllerService> nfcControllerService_;
    static std::mutex mutex_;
};
} // namespace KITS
} // namespace NFC
} // namespace OHOS
#endif // NFC_CONTROLLER_H
