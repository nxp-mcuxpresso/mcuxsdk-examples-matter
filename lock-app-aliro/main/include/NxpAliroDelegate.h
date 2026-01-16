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

#include "aliro_reader_api.h"

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
    static constexpr uint8_t kMaxNxpAliroBleUwbProtVersions = 4; // Max number of supported protocol versions
    static constexpr uint16_t kNxpAliroBleUwbProtVersion = 0x0100;
    static constexpr size_t kMaxNxpAliroExpeditedTransSupProtVersions = 4; // Max number of supported protocol versions
    static constexpr uint16_t kNxpAliroExpeditedTransSupProtVersion = 0x0100;
    static constexpr uint8_t kNxpAliroNumberOfEndpointKeysSupported = 10;
    static constexpr uint8_t kNxpAliroNumberOfCredentialIssuerKeysSupported = 10;

    ///TODO: The value should come from ALIRO side
    static constexpr uint8_t kNxpAliroPublicKeySize = 65;
    static constexpr uint8_t kNxpAliroPrivateKeySize = 32;
    static constexpr uint8_t kNxpAliroGroupIdentifierSize = 16;
    static constexpr uint8_t kNxpAliroGroupResolvingKeySize = 16;
    static constexpr uint8_t kNxpAliroUserKeyStringSize = 20;

    static constexpr char kNxpAliroPublicKeyString[] = "nxp/aliro/pub";
    static constexpr char kNxpAliroPrivateKeyString[] = "nxp/aliro/priv";
    static constexpr char kNxpAliroGroupIdentifierString[] = "nxp/aliro/gId";
    static constexpr char kNxpAliroGroupResolvingKeyString[] = "nxp/aliro/gRes";
    static constexpr char kNxpAliroCredIsuerKeyString[] = "nxp/aliro/isuer";
    static constexpr char kNxpAliroUserKeyString[] = "nxp/aliro/user";
    static constexpr char kNxpAliroSeedString[] = "NXP_ALIRO_SEED_2025";

    // static constexpr uint32_t kNxpAliroCapabilities = ALIRO_CAP_BLE | ALIRO_CAP_UWB | ALIRO_CAP_NFC | ALIRO_CAP_KEY_SLOT |
    //                                                   ALIRO_CAP_FAST_TRANSACTION | ALIRO_CAP_STEP_UP | ALIRO_CAP_HW_RNG;
    static constexpr uint32_t kNxpAliroCapabilities = 1;

    static NxpAliroDelegate & Instance();

    // Aliro credential configuration method
    bool SetAliroCredential(chip::EndpointId endpointId, uint16_t credentialIndex,
                            chip::FabricIndex creator, chip::FabricIndex modifier,
                            DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                            const chip::ByteSpan & credentialData);

    bool IsDoorUnlocked();
    void SetDoorLockState(bool locked);

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
    template<size_t N>
    struct VersionCache
    {
        size_t nbOfEntries = 0;
        uint16_t versionEntry[N];
        
        VersionCache() : nbOfEntries(0)
        {
            memset(versionEntry, 0, sizeof(versionEntry));
        }

        constexpr size_t capacity() const { return N; }

        void clear()
        {
            nbOfEntries = 0;
            memset(versionEntry, 0, sizeof(versionEntry));
        }
    };

    // Initialization state
    bool mAliroStateInitialized = false;
    bool mAliroTransportInitialized = false;

    // ALIRO versions cache
    VersionCache<kMaxNxpAliroBleUwbProtVersions> mBleUwbVersionCache;               // For BLE/UWB protocol versions
    VersionCache<kMaxNxpAliroExpeditedTransSupProtVersions> mExpeditedVersionCache; // For expedited transaction versions

    // Helper methods
    CHIP_ERROR CopyProtocolVersionIntoSpan(uint16_t protocolVersionValue, MutableByteSpan & protocolVersion);
    CHIP_ERROR SetAliroReaderConfigInternal(const uint8_t *signingKey, 
                                            const uint8_t *verificationKey,
                                            const uint8_t *groupIdentifier,
                                            const uint8_t *groupResolvingKey);
    void SaveAliroReaderConfig(const uint8_t *signingKey,
                               const uint8_t *verificationKey,
                               const uint8_t *groupIdentifier,
                               const uint8_t *groupResolvingKey);
    CHIP_ERROR RestoreAliroUserConfig(uint8_t *publicKey);
    CHIP_ERROR ClearAliroUserConfig();

    void EnableAliroHwConfig();
    void StartAliroTasks();

    static const char * GetAliroCredentialTypeString(CredentialTypeEnum credentialType);
    void CheckAndInitTransportLayer();
    void AliroReaderCallback(uint8_t ins, void *param);
    void HandleInterfaceStatus(int32_t status);

    static NxpAliroDelegate instance;
};

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip