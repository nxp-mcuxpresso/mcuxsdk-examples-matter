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
 *   Application-level override of DiagnosticDataProviderImpl for the NXP
 *   thermostat example.
 *
 *   Subclasses the common NXP DiagnosticDataProviderImpl and overrides
 *   GetActiveHardwareFaults() to add thermostat-specific hardware fault
 *   detection (e.g. temperature sensor faults).
 *
 */

#pragma once

#include <platform/nxp/common/DiagnosticDataProviderImpl.h>

namespace chip {
namespace DeviceLayer {

class ThermostatDiagnosticDataProvider : public DiagnosticDataProviderImpl
{
public:
    /**
     * Populate @p hardwareFaults with all currently active hardware faults.
     *
     * Delegates to the platform-level injectable fault flags (via
     * chip::DeviceLayer::NXP::GetPlatformHardwareFaults) and then appends any
     * thermostat-specific fault conditions detected at application level.
     */
    CHIP_ERROR GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults) override;
};

} // namespace DeviceLayer
} // namespace chip