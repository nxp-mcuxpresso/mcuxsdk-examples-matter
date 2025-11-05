/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2025 NXP
  *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "AppTaskAliro.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/data-model/Nullable.h>

#include "aliro_core.h"
#include "NxpAliroDelegate.h"

#if !CHIP_CONFIG_ENABLE_ICD_SERVER
#include "ICDUtil.h"
#endif

#ifndef APP_DEVICE_TYPE_ENDPOINT
#define APP_DEVICE_TYPE_ENDPOINT 1
#endif

bool bAliroOperation = false;

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;
using namespace chip::app::DataModel;

namespace LockApp {

void AppTaskAliro::AppMatter_AliroStateMachine(aliro_transport_interface_type_t aTransport, aliro_reader_status_state_t aReaderState)
{
    Nullable<DoorLock::DlLockState> state(DlLockState::kUnlocked);
    DoorLock::Attributes::LockState::Get(APP_DEVICE_TYPE_ENDPOINT, state);

    switch (state.Value())
    {
        case DlLockState::kUnlocked:
        case DlLockState::kUnlatched:
        case DlLockState::kNotFullyLocked:
            if (aReaderState == kAliro_ReaderStatusState_Secured)
            {
                bAliroOperation = true;
                DoorLockServer::Instance().SetLockState(1, DlLockState::kLocked, OperationSourceEnum::kAliro, NullNullable, NullNullable,
                                                        NullNullable, NullNullable);
            }
        break;

        case DlLockState::kLocked:
            if (aReaderState == kAliro_ReaderStatusState_Unsecured)
            {
                bAliroOperation = true;
                DoorLockServer::Instance().SetLockState(1, DlLockState::kUnlocked, OperationSourceEnum::kAliro, NullNullable, NullNullable,
                                                        NullNullable, NullNullable);
            }
        break;

        default:
        break;
    }
}

void AppTaskAliro::PostInitMatterStack()
{
    // Call parent implementation first
    AppTask::PostInitMatterStack();

    // Set up Aliro-specific callback using a lambda to bind to this instance
    aliro_set_door_lock_external_cb(AppTaskAliro::AppMatter_AliroStateMachine);

    NxpAliroDelegate::Instance().RestoreAliroReaderConfig();
}

// This returns an instance of this class.
AppTaskAliro & AppTaskAliro::GetDefaultInstance()
{
    static AppTaskAliro sAppTaskAliro;
    return sAppTaskAliro;
}

} // namespace LockApp

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return LockApp::AppTaskAliro::GetDefaultInstance();
}
