/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef NFC_NAPI_UTILS_H_
#define NFC_NAPI_UTILS_H_

#include <chrono>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "nfc_sdk_common.h"

namespace OHOS {
namespace NFC {
namespace KITS {
class AsyncContext {
public:
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    std::function<void(void *)> executeFunc;
    std::function<void(void *)> completeFunc;
    napi_value resourceName;
    napi_value result;
    int errorCode;

    AsyncContext(napi_env e, napi_async_work w = nullptr, napi_deferred d = nullptr)
    {
        env = e;
        work = w;
        deferred = d;
        executeFunc = nullptr;
        completeFunc = nullptr;
        result = nullptr;
        errorCode = NfcErrorCode::NFC_SUCCESS;
    }

    AsyncContext() = delete;

    virtual ~AsyncContext() {}
};

class ReadAsyncContext : public AsyncContext {
public:
    std::string respNdefData;
    ReadAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    ReadAsyncContext() = delete;

    ~ReadAsyncContext() override {}
};

enum class JS_CALLBACK_ARGV {
    CALLBACK_ARGV_INDEX_0 = 0,
    CALLBACK_ARGV_INDEX_1,
    CALLBACK_ARGV_CNT,
};

enum class JS_ARGV_NUM {
    ARGV_NUM_0 = 0,
    ARGV_NUM_1 = 1,
    ARGV_NUM_2 = 2,
    ARGV_NUM_3 = 3,
    ARGV_NUM_4 = 4,
    ARGV_NUM_5 = 5,
};

enum class JS_ARGV_INDEX {
    ARGV_INDEX_0 = 0,
    ARGV_INDEX_1,
    ARGV_INDEX_2,
    ARGV_INDEX_3,
    ARGV_INDEX_4,
};

struct NfcAsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    int32_t result;
    int32_t uid = 0;
    bool flag = false;
};

enum NapiError : int32_t {
    ERROR_NONE = 0,
    ERROR_DEFAULT = -1,
    ERROR_SERVICE_UNAVAILABLE = -2,
    ERROR_PARAMETER_VALUE_INVALID = -3,
    ERROR_PARAMETER_COUNTS_INVALID = -4,
    ERROR_PARAMETER_TYPE_INVALID = -5,
    ERROR_NATIVE_API_EXECUTE_FAIL = -6,
};

struct BaseContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    bool resolved = false;
    int32_t errorCode = ERROR_DEFAULT;
};

template<typename T, typename D>
struct CallBackContext : BaseContext {
    T value;
    D *objectInfo;
};

std::vector<std::string> ConvertStringVector(napi_env env, napi_value jsValue);
napi_value CreateErrorMessage(napi_env env, std::string message, int32_t errorCode = ERROR_DEFAULT);
napi_value CreateUndefined(napi_env env);
std::string GetNapiStringValue(
    napi_env env, napi_value napiValue, const std::string &name, const std::string &defValue = "");
std::string GetStringFromValue(napi_env env, napi_value value);
napi_value GetNamedProperty(napi_env env, napi_value object, const std::string &propertyName);
int32_t GetNapiInt32Value(napi_env env, napi_value napiValue, const std::string &name, const int32_t &defValue = 0);
std::string UnwrapStringFromJS(napi_env env, napi_value param);
void ConvertStringVectorToJS(napi_env env, napi_value result, std::vector<std::string> &stringVector);
void ConvertIntVectorToJS(napi_env env, napi_value result, std::vector<int> &intVector);
bool MatchParameters(napi_env env, const napi_value parameters[], std::initializer_list<napi_valuetype> valueTypes);
napi_value HandleAsyncWork(napi_env env, BaseContext *context, const std::string &workName,
    napi_async_execute_callback execute, napi_async_complete_callback complete);
void Handle1ValueCallback(napi_env env, BaseContext *context, napi_value callbackValue);
void Handle2ValueCallback(napi_env env, BaseContext *context, napi_value callbackValue);
} // namespace KITS
} // namespace NFC
} // namespace OHOS
#endif