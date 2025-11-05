/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/clusters/door-lock-server/door-lock-delegate.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

// using chip::app::Clusters::DoorLock::DoorLockServer::DlCredentialStatus;

/**
 * @brief NXP implementation of the Aliro door lock delegate.
 * 
 * This class provides implementations for all Aliro-related functionality
 * in the door lock cluster. Applications can override specific methods as needed.
 */
class NxpAliroDelegate : public Delegate
{
public:
    NxpAliroDelegate() = default;
    ~NxpAliroDelegate() override = default;

    static constexpr uint8_t kNxpAliroBLEAdvertisingVersion = 0; // For now the only define valuse of the BLE advertising version for Aliro is 0.
    static constexpr uint8_t kMaxNxpAliroBleUwbProtVersions = 1; // Max number of supported protocol versions
    static constexpr uint16_t kNxpAliroBleUwbProtVersion = 0x0100;
    static constexpr size_t kMaxNxpAliroExpeditedTransSupProtVersions = 1; // Max number of supported protocol versions
    static constexpr uint16_t kNxpAliroExpeditedTransSupProtVersion = 0x0100;

    static constexpr char kNxpAliroPublicKeyString[] = "nxp/aliro/pub";
    static constexpr char kNxpAliroPrivateKeyString[] = "nxp/aliro/priv";
    static constexpr char kNxpAliroGroupIdentifierString[] = "nxp/aliro/gId";
    static constexpr char kNxpAliroGroupResolvingKeyString[] = "nxp/aliro/gRes";
    static constexpr char kNxpAliroCredIsuerKeyString[] = "nxp/aliro/cIss";
    static constexpr char kNxpAliroUserKeyString[] = "nxp/aliro/user";

    static NxpAliroDelegate & Instance();

    // Aliro credential configuration method
    bool SetAliroCredential(chip::EndpointId endpointId, uint16_t credentialIndex,
                            chip::FabricIndex creator, chip::FabricIndex modifier,
                            DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                            const chip::ByteSpan & credentialData);

    bool GetAliroCredential(chip::EndpointId endpointId, uint16_t credentialIndex, 
                           CredentialTypeEnum credentialType, EmberAfPluginDoorLockCredentialInfo & credential);

    // Aliro Reader Configuration Methods
    CHIP_ERROR GetAliroReaderVerificationKey(MutableByteSpan & verificationKey) override;
    CHIP_ERROR GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier) override;
    CHIP_ERROR GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier) override;
    CHIP_ERROR GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey) override;

    // Aliro Protocol Version Methods
    CHIP_ERROR GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
                                                                           MutableByteSpan & protocolVersion) override;
    CHIP_ERROR GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) override;

    // Aliro Capability Methods
    uint8_t GetAliroBLEAdvertisingVersion() override;
    uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() override;
    uint16_t GetNumberOfAliroEndpointKeysSupported() override;

    // Aliro Configuration Methods
    CHIP_ERROR SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey,
                                    const ByteSpan & groupIdentifier, const Optional<ByteSpan> & groupResolvingKey) override;
    CHIP_ERROR ClearAliroReaderConfig() override;

    // Restores ALIRO reader configuration from persistent storage
    CHIP_ERROR RestoreAliroReaderConfig();

private:
    CHIP_ERROR CopyProtocolVersionIntoSpan(uint16_t protocolVersionValue, MutableByteSpan & protocolVersion);
    void SaveAliroReaderConfig();
    void EnableAliroHwConfig();
    void StartAliroTasks();

    static const char * GetAliroCredentialTypeString(CredentialTypeEnum credentialType);

    bool mAliroStateInitialized = false;

    static NxpAliroDelegate instance;
};

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip