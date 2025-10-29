/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    Copyright 2025 NXP
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

#include "AppTask.h"
#include "AppTaskCs.h"

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include "BLEApplicationManager.h"
#endif

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::NXP::App;

static CHIP_ERROR cliBleCsAdv(int argc, char * argv[])
{
    chip::NXP::App::BleAppMgr().EnableMultipleConnectionsHandler();

    return CHIP_NO_ERROR;
}

void LightSwitchAppBleCs::AppTaskCs::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    static const shell_command_t kCommands[] = {
        {
            .cmd_func = cliBleCsAdv,
            .cmd_name = "startblecsadv",
            .cmd_help = "Start BLE advertisement for BLE CS Distance Measurement "
        },
    };

    LightSwitchApp::AppTask::AppMatter_RegisterCustomCliCommands();

    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

LightSwitchAppBleCs::AppTaskCs & LightSwitchAppBleCs::AppTaskCs::GetDefaultInstance()
{
    static LightSwitchAppBleCs::AppTaskCs sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return LightSwitchAppBleCs::AppTaskCs::GetDefaultInstance();
}
