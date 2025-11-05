/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

/**
 * @file DeviceCallbacksAliro.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack with Aliro support
 *
 **/
#include "DeviceCallbacksAliro.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/Dnssd.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>

#include <lib/support/CodeUtils.h>

#include "aliro_core.h"

extern bool bAliroOperation;

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;

namespace LockApp {

DeviceCallbacksAliro DeviceCallbacksAliro::sDeviceCallbacksAliro;

void DeviceCallbacksAliro::PostAttributeChangeCallback(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                                       uint8_t type, uint16_t size, uint8_t * value)
{
    // Call parent implementation first
    DeviceCallbacks::PostAttributeChangeCallback(endpointId, clusterId, attributeId, type, size, value);

    if ((clusterId == DoorLock::Id) && (attributeId == Attributes::LockState::Id))
    {
        // Handle lock state attribute change, inform ALIRO when a door lock state change happens that was not trriggered by ALIRO itself
        // If Aliro operation flag is set (bAliroOperation), it means this change was triggered by ALIRO, so skip further processing
        DlLockState lockState = static_cast<DlLockState>(*value);
        if (bAliroOperation == true)
        {
            bAliroOperation = false;
        }
        else
        {
            // Perform actions based on lock state
            switch (lockState)
            {
                case DlLockState::kLocked:
                    aliro_handle_external_door_lock_event(kAliro_ReaderStatusState_Secured);
                    break;

                case DlLockState::kUnlocked:
                case DlLockState::kUnlatched:
                    aliro_handle_external_door_lock_event(kAliro_ReaderStatusState_Unsecured);
                    break;
                default:
                    ChipLogError(DeviceLayer, "Invalid lock state received");
                    break;
            }
        }
    }
}

// This returns an instance of this class.
DeviceCallbacksAliro & DeviceCallbacksAliro::GetDefaultInstance()
{
    static DeviceCallbacksAliro sDeviceCallbacksAliro;
    return sDeviceCallbacksAliro;
}

} // namespace LockApp

chip::DeviceManager::CHIPDeviceManagerCallbacks & chip::NXP::App::GetDeviceCallbacks()
{
    return LockApp::DeviceCallbacksAliro::GetDefaultInstance();
}
