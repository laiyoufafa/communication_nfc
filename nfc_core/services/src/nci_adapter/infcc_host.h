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
#ifndef I_NFCC_HOST_H
#define I_NFCC_HOST_H

#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace NFC {
namespace NCI {
class INfccHost {
public:
    class INfccHostListener {
    public:
        virtual ~INfccHostListener() {}
    };

    virtual ~INfccHost() {}
    virtual void SetNfccHostListener(std::weak_ptr<INfccHostListener> listener) = 0;
    /**
     * @brief Turn on NFC
     * @return True if ok
     */
    virtual bool Initialize() = 0;
    /**
     * @brief Turn off NFC
     * @return True if ok
     */
    virtual bool Deinitialize() = 0;
    /**
     * @brief Start polling and listening
     * @param techMask bitmask of the technologies
     * @param enableReaderMode if enable tag polling
     * @param enableHostRouting if enable host routing
     * @param restart if need restart
     */
    virtual void EnableDiscovery(int techMask, bool enableReaderMode, bool enableHostRouting, bool restart) = 0;
    /**
     * @brief Stop polling and listening
     */
    virtual void DisableDiscovery() = 0;
    /**
     * @brief Send a raw frame
     * @param rawData raw frame
     * @return True if ok
     */
    virtual bool SendRawFrame(std::string& rawData) = 0;
    /**
     * @brief Send screen statue
     * @param screenStateMask bitmask of the screen state
     * @return True if ok
     */
    virtual bool SetScreenStatus(unsigned char screenStateMask) = 0;
    /**
     * @brief Get Nci version
     * @return Nci version
     */
    virtual int GetNciVersion() = 0;
    /**
     * @brief Set Secure Nfc
     * @param secure if need secure nfc
     * @return True if ok
     */
    virtual bool SetSecureNfc(bool secure) = 0;
    /**
     * @brief Get maximum tranceive length
     * @return max length
     */
    virtual int GetIsoDepMaxTransceiveLength() = 0;
    virtual int RegisterT3tIdentifier(std::string& t3tIdentifier) = 0;
    virtual void DeregisterT3tIdentifier(std::string& t3tIdentifier) = 0;
    virtual void ClearT3tIdentifiersCache() = 0;
    virtual int GetLfT3tMax() = 0;
    virtual int GetLastError() = 0;
    virtual void Abort() = 0;
    /**
     * @brief Download firmware
     * @return True if ok
     */
    virtual bool CheckFirmware() = 0;
    virtual void Dump(int fd) = 0;
    virtual void FactoryReset() = 0;
    virtual void Shutdown() = 0;
    virtual bool AddAidRouting(std::string& aid, int route, int aidInfo) = 0;
    virtual bool RemoveAidRouting(std::string& aid) = 0;
    virtual bool CommitRouting() = 0;
    virtual int GetAidRoutingTableSize() = 0;
    virtual int GetDefaultRoute() = 0;
    virtual int GetDefaultOffHostRoute() = 0;
    virtual std::vector<int> GetOffHostUiccRoute() = 0;
    virtual std::vector<int> GetOffHostEseRoute() = 0;
    virtual int GetAidMatchingMode() = 0;
    virtual int GetDefaultIsoDepRouteDestination() = 0;
    virtual bool CanMakeReadOnly(int ndefType) = 0;
    virtual bool GetExtendedLengthApdusSupported() = 0;
    virtual bool ClearAidTable() = 0;
    virtual int GetRemainRoutingTableSize() = 0;
};
}  // namespace NCI
}  // namespace NFC
}  // namespace OHOS
#endif  // I_NFCC_HOST_H
