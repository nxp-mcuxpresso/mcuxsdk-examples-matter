/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2026 NXP
 *    All rights reserved.
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
 * @file
 *   Application-level implementation of GetActiveHardwareFaults() for the NXP
 *   thermostat example.
 *
 *   How to extend:
 *   - Implement GetActiveHardwareFaults() to report active hardware faults
 *     relevant to your application by appending fault values to the
 *     hardwareFaults list using hardwareFaults.add().
 *   - Use the HardwareFaultType enum values defined in the Matter spec
 *     (e.g. kSensor, kPowerSource) to identify fault categories.
 *   - Return CHIP_NO_ERROR on success
 */

#include "ThermostatDiagnosticDataProvider.h"

namespace chip {
namespace DeviceLayer {


DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    static ThermostatDiagnosticDataProvider sInstance;
    return sInstance;
}

CHIP_ERROR ThermostatDiagnosticDataProvider::GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults)
{
    /* Add custom implementation for active hardware faults */

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip