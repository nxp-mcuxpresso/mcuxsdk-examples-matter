/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "NxpAliroDelegate.h"

#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

#include <platform/nxp/common/NXPConfig.h>
#include <platform/ConnectivityManager.h>
#include <platform/nxp/common/ble/BLEManagerCommon.h>

#include "aliro_keys.h"
#include "aliro_nfc.h"
#include "aliro_ble.h"
#include "app_conn.h"
#include "aliro/board/pin_mux.h"

#include "AppInternal.h"
#include "AppRecovery.h"

using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

 
NxpAliroDelegate NxpAliroDelegate::instance;

NxpAliroDelegate & NxpAliroDelegate::Instance()
{
    return instance;
}

CHIP_ERROR NxpAliroDelegate::GetAliroReaderVerificationKey(MutableByteSpan & verificationKey)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroReaderVerificationKey");

    if (!mAliroStateInitialized)
    {
        // Set to size 0 to indicate no value
        verificationKey.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    return chip::CopySpanToMutableSpan(ByteSpan(reader_public_key), verificationKey);
}

CHIP_ERROR NxpAliroDelegate::GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroReaderGroupIdentifier");

    if (!mAliroStateInitialized)
    {
        // Set to size 0 to indicate no value
        groupIdentifier.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    return CopySpanToMutableSpan(ByteSpan(READER_GROUP_IDENTIFIER), groupIdentifier);
}

CHIP_ERROR NxpAliroDelegate::GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroReaderGroupSubIdentifier");

    return CopySpanToMutableSpan(ByteSpan(READER_SUB_GROUP_IDENTIFIER), groupSubIdentifier);
}

CHIP_ERROR NxpAliroDelegate::GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroGroupResolvingKey");

    if (!mAliroStateInitialized)
    {
        // Set to size 0 to indicate no value
        groupResolvingKey.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    return CopySpanToMutableSpan(ByteSpan(GROUP_RESOLVING_KEY), groupResolvingKey);
}

CHIP_ERROR NxpAliroDelegate::GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
                                                                                         MutableByteSpan & protocolVersion)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex, index: %u", 
                    static_cast<unsigned>(index));

    if (index > kMaxNxpAliroExpeditedTransSupProtVersions - 1)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return CopyProtocolVersionIntoSpan(kNxpAliroExpeditedTransSupProtVersion, protocolVersion);
}

CHIP_ERROR NxpAliroDelegate::GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroSupportedBLEUWBProtocolVersionAtIndex, index: %u", 
                    static_cast<unsigned>(index));

    if (index > kMaxNxpAliroBleUwbProtVersions - 1)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return CopyProtocolVersionIntoSpan(kNxpAliroBleUwbProtVersion, protocolVersion);
}

uint8_t NxpAliroDelegate::GetAliroBLEAdvertisingVersion()
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroBLEAdvertisingVersion");
    
    return kNxpAliroBLEAdvertisingVersion;
}

uint16_t NxpAliroDelegate::GetNumberOfAliroCredentialIssuerKeysSupported()
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetNumberOfAliroCredentialIssuerKeysSupported");

    return 10;
}

uint16_t NxpAliroDelegate::GetNumberOfAliroEndpointKeysSupported()
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetNumberOfAliroEndpointKeysSupported");

    return 10;
}

CHIP_ERROR NxpAliroDelegate::SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey,
                                                  const ByteSpan & groupIdentifier, const Optional<ByteSpan> & groupResolvingKey)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::SetAliroReaderConfig");
    ChipLogProgress(DeviceLayer, "  signingKey length: %u", static_cast<unsigned>(signingKey.size()));
    ChipLogProgress(DeviceLayer, "  verificationKey length: %u", static_cast<unsigned>(verificationKey.size()));
    ChipLogProgress(DeviceLayer, "  groupIdentifier length: %u", static_cast<unsigned>(groupIdentifier.size()));
    ChipLogProgress(DeviceLayer, "  groupResolvingKey present: %s", groupResolvingKey.HasValue() ? "yes" : "no");
    
    VerifyOrReturnError(signingKey.size() == sizeof(reader_private_key), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(reader_private_key, signingKey.data(), signingKey.size());

    VerifyOrReturnError(verificationKey.size() == sizeof(reader_public_key), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(reader_public_key, verificationKey.data(), verificationKey.size());

    VerifyOrReturnError(groupIdentifier.size() == sizeof(READER_GROUP_IDENTIFIER), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(READER_GROUP_IDENTIFIER, groupIdentifier.data(), groupIdentifier.size());

    if (groupResolvingKey.HasValue())
    {
        VerifyOrReturnError(groupResolvingKey.Value().size() == sizeof(GROUP_RESOLVING_KEY), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(GROUP_RESOLVING_KEY, groupResolvingKey.Value().data(), groupResolvingKey.Value().size());
    }

    if (mAliroStateInitialized == false)
    {
        EnableAliroHwConfig();
    }

    SaveAliroReaderConfig();
    mAliroStateInitialized = true;
 
    return CHIP_NO_ERROR;
}

CHIP_ERROR NxpAliroDelegate::ClearAliroReaderConfig()
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::ClearAliroReaderConfig");

    if (mAliroStateInitialized)
    {
        // Clear all Aliro configuration keys from persistent storage
        NXPConfig::ClearConfigValue(kNxpAliroPublicKeyString);
        NXPConfig::ClearConfigValue(kNxpAliroPrivateKeyString);
        NXPConfig::ClearConfigValue(kNxpAliroGroupIdentifierString);
        NXPConfig::ClearConfigValue(kNxpAliroGroupResolvingKeyString);

        // Clear in-memory keys
        memset(reader_public_key, 0, sizeof(reader_public_key));
        memset(reader_private_key, 0, sizeof(reader_private_key));
        memset(READER_GROUP_IDENTIFIER, 0, sizeof(READER_GROUP_IDENTIFIER));
        memset(GROUP_RESOLVING_KEY, 0, sizeof(GROUP_RESOLVING_KEY));

        mAliroStateInitialized = false;
        ///TODO: Stop BLE advertising and UWB
    }

    return CHIP_NO_ERROR;
}

bool NxpAliroDelegate::SetAliroCredential(chip::EndpointId endpointId, uint16_t credentialIndex,
                                          chip::FabricIndex creator, chip::FabricIndex modifier,
                                          DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                          const chip::ByteSpan & credentialData)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::HandleAliroCredentialSet");
    ChipLogProgress(DeviceLayer, "  CredentialType: %s", GetAliroCredentialTypeString(credentialType));
    ChipLogProgress(DeviceLayer, "  CredentialIndex: %u", credentialIndex);

    bool result = false;

    // Validate issuer key size (typically public key size)
    if (credentialData.size() != ALIRO_CRYPTO_EC_PUBKEY_DER_LEN)
    {
        ChipLogError(DeviceLayer, "Invalid Aliro Credential Key size: %u, expected: %u", 
                    static_cast<unsigned>(credentialData.size()), ALIRO_CRYPTO_EC_PUBKEY_DER_LEN);
        ExitNow();
    }

    // Validate credential data size based on type
    switch (credentialType)
    {
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
        if (credentialIndex == 1)
        {
            memcpy(credential_issuer_public_key, credentialData.data(), ALIRO_CRYPTO_EC_PUBKEY_DER_LEN);
            NXPConfig::WriteConfigValueBin(kNxpAliroCredIsuerKeyString, credential_issuer_public_key, ALIRO_CRYPTO_EC_PUBKEY_DER_LEN);
        }
        break;

    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        if (credentialIndex == 1)
        {
            memcpy(access_credential_public_key, credentialData.data(), sizeof(access_credential_public_key));
            NXPConfig::WriteConfigValueBin(kNxpAliroUserKeyString, access_credential_public_key, sizeof(access_credential_public_key));

            StartAliroTasks();
        }
        break;

    default:
        ChipLogError(DeviceLayer, "Unknown Aliro credential type: %u", static_cast<uint8_t>(credentialType));
        return false;
    }

    // If this is the first credential being set and Aliro state is not initialized,
    // we might need to initialize some Aliro subsystems
    if (!mAliroStateInitialized)
    {
        ChipLogProgress(DeviceLayer, "First Aliro credential set, but reader config not initialized yet");
        // Note: Full Aliro functionality requires reader configuration to be set first
    }

    result = true;
    ChipLogProgress(DeviceLayer, "Aliro credential stored successfully");

exit:
    return result;
}

// PRIVATE methods

CHIP_ERROR NxpAliroDelegate::CopyProtocolVersionIntoSpan(uint16_t protocolVersionValue, MutableByteSpan & protocolVersion)
{
    static_assert(sizeof(protocolVersionValue) == kAliroProtocolVersionSize);

    if (protocolVersion.size() < kAliroProtocolVersionSize)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Per Aliro spec, protocol version encoding is big-endian
    chip::Encoding::BigEndian::Put16(protocolVersion.data(), protocolVersionValue);
    protocolVersion.reduce_size(kAliroProtocolVersionSize);
    return CHIP_NO_ERROR;
}

const char * NxpAliroDelegate::GetAliroCredentialTypeString(CredentialTypeEnum credentialType)
{
    switch (credentialType)
    {
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
        return "AliroCredentialIssuerKey";
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
        return "AliroEvictableEndpointKey";
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        return "AliroNonEvictableEndpointKey";
    default:
        return "Generic Aliro Credential Type";
    }
}

void NxpAliroDelegate::SaveAliroReaderConfig()
{
    NXPConfig::WriteConfigValueBin(kNxpAliroPublicKeyString, reader_public_key, sizeof(reader_public_key));
    NXPConfig::WriteConfigValueBin(kNxpAliroPrivateKeyString, reader_private_key, sizeof(reader_private_key));
    NXPConfig::WriteConfigValueBin(kNxpAliroGroupIdentifierString, READER_GROUP_IDENTIFIER, sizeof(READER_GROUP_IDENTIFIER));
    NXPConfig::WriteConfigValueBin(kNxpAliroGroupResolvingKeyString, GROUP_RESOLVING_KEY, sizeof(GROUP_RESOLVING_KEY));
}

CHIP_ERROR NxpAliroDelegate::RestoreAliroReaderConfig()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t bytesRead = 0;

    // Check the existence of the public key before try to restore the full ALIRO configuration
    err = NXPConfig::ReadConfigValueBin(kNxpAliroPublicKeyString, reader_public_key, sizeof(reader_public_key), bytesRead);
    VerifyOrReturnError(err == CHIP_NO_ERROR && bytesRead == sizeof(reader_public_key), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Restore signing key (private key)
    err = NXPConfig::ReadConfigValueBin(kNxpAliroPrivateKeyString, reader_private_key, sizeof(reader_private_key), bytesRead);
    VerifyOrReturnError(err == CHIP_NO_ERROR && bytesRead == sizeof(reader_private_key), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    
    // Restore group identifier
    err = NXPConfig::ReadConfigValueBin(kNxpAliroGroupIdentifierString, READER_GROUP_IDENTIFIER, sizeof(READER_GROUP_IDENTIFIER), bytesRead);
    VerifyOrReturnError(err == CHIP_NO_ERROR && bytesRead == sizeof(READER_GROUP_IDENTIFIER), CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    // Restore group resolving key
    err = NXPConfig::ReadConfigValueBin(kNxpAliroGroupResolvingKeyString, GROUP_RESOLVING_KEY, sizeof(GROUP_RESOLVING_KEY), bytesRead);
    VerifyOrReturnError(err == CHIP_NO_ERROR && bytesRead == sizeof(GROUP_RESOLVING_KEY), CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    // Restore user keys
    err = NXPConfig::ReadConfigValueBin(kNxpAliroUserKeyString, access_credential_public_key, sizeof(access_credential_public_key), bytesRead);
    VerifyOrReturnError(err == CHIP_NO_ERROR && bytesRead == sizeof(access_credential_public_key), CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    err = NXPConfig::ReadConfigValueBin(kNxpAliroCredIsuerKeyString, credential_issuer_public_key, ALIRO_CRYPTO_EC_PUBKEY_DER_LEN, bytesRead);
    VerifyOrReturnError(err == CHIP_NO_ERROR && bytesRead == ALIRO_CRYPTO_EC_PUBKEY_DER_LEN, CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    mAliroStateInitialized = true;

    EnableAliroHwConfig();
    StartAliroTasks();

    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::RestoreAliroReaderConfig sucessfull");

    return CHIP_NO_ERROR;
}

void NxpAliroDelegate::EnableAliroHwConfig()
{
    BOARD_InitAliroPins();
}

void NxpAliroDelegate::StartAliroTasks()
{
    AppSemaphoresAndMutextes_Init();

    /* Register BLE application callbacks*/
    BLEMgrImpl().RegisterAppCallbacks(nullptr, nullptr, App_GenericCallback, nullptr);

    StartTask_UWBRecovery();
    StartTask_BLE();
    StartTask_NFC();
}

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip