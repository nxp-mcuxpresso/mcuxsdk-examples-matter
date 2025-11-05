/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2021-2023 Google LLC.
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

#pragma once

#include "AppTask.h"
#include "aliro_core.h"

namespace LockApp {

class AppTaskAliro : public AppTask
{
public:
    ~AppTaskAliro() override = default;
    
    void PostInitMatterStack(void) override;
    
    // This returns an instance of this class.
    static AppTaskAliro & GetDefaultInstance();
    static void AppMatter_AliroStateMachine(aliro_transport_interface_type_t aTransport, aliro_reader_status_state_t aReaderState);
private:

    static AppTaskAliro sAppTaskAliro;
};

} // namespace LockApp
