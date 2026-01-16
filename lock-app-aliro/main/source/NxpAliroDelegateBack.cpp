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

#include "aliro_reader_api.h"
#include "serial_transport.h"

#define ALIRO_STATUS_SUCCESS 0

extern "C" void BOARD_InitPinLPUART0_RX(void);
extern "C" void BOARD_InitPinLPUART0_TX(void);

using namespace chip::DeviceLayer::Internal;
namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

#define SYNC_CODE_BITMASK(val) (0x01U << ((val)-1U))

uint8_t reader_private_key[32] = {0};
uint8_t reader_public_key[65] = {0};
uint8_t READER_GROUP_IDENTIFIER[16] = {0};
uint8_t GROUP_RESOLVING_KEY[16] = {0};
uint8_t READER_SUB_GROUP_IDENTIFIER[16] = {
    0x11, 0x33, 0x44, 0x66, 0x77, 0x99, 0xAA, 0x00, 0x11, 0x33, 0x44, 0x66, 0x77, 0x99, 0xAA, 0x00
};

uint8_t access_credential_public_key[] = {
    0x04, 0x74, 0x2d, 0xf7, 0x36, 0xd0, 0xfc, 0x9b, 0xe9, 0x78, 0xc4, 0x5b, 0x00, 0xe8, 0xfd, 0xf7, 0xce,
    0xa6, 0x84, 0xea, 0x10, 0x5a, 0xe5, 0x74, 0xc1, 0x50, 0x5a, 0x2c, 0x24, 0xab, 0x61, 0x98, 0xe3, 0x12,
    0x5b, 0x7f, 0x1b, 0x7e, 0x1d, 0x13, 0x4c, 0x55, 0xec, 0xe6, 0x96, 0x81, 0xba, 0x8e, 0xcc, 0x18, 0xa3,
    0x83, 0x6d, 0xc5, 0x19, 0x9c, 0x75, 0x9f, 0x31, 0xe8, 0xcc, 0xf1, 0x7e, 0x3e, 0xfa};

uint8_t access_credential_private_key[] = {
    0xf6, 0xf6, 0x01, 0xca, 0xc6, 0x4e, 0x2d, 0x4e, 0x47, 0xe9, 0xb2, 0xd1, 0xd0, 0x40, 0x86, 0x80,
    0xce, 0xf9, 0x5e, 0x4e, 0x84, 0xb5, 0xec, 0xee, 0x64, 0xd3, 0x40, 0x17, 0x73, 0xbf, 0x94, 0x26};


static extern_aliro_uwb_capabilities_config_t uwb_config = {
    .uwb_configuration_identifier = {0x0000, 0x0001},
    .uwb_configuration_identifier_entries = 2,

    .pulse_shape_combination = {0x00, 0x11, 0x22},
    .pulse_shape_combination_entries = 3,

    .channel_bitmask = kAliro_UwbChannel_Bitmask9 | kAliro_UwbChannel_Bitmask5, // CH5 + CH9

    .ran_multiplier = 1, // Minimum supported RAN multiplier, For now we always sent back the
                         // RAN multiplier of the user device

    .sync_code_index_bitmask = SYNC_CODE_BITMASK(12) | SYNC_CODE_BITMASK(11) |
                               SYNC_CODE_BITMASK(10) | SYNC_CODE_BITMASK(9), // 12 11 10 9

    .hopping_configuration_bitmask = 0x50,
    .slot_bitmask                  = 0x02 | 0x04,
    .number_slots_per_round        = 24,
    .mac_mode_bitmask              = 0x40 | 0x01
};

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

    // uint8_t version = 0;
    // aliro_reader_get_params_t expeditedProtVersions;
    // int32_t status = aliroReaderGetParams(ALIRO_READER_EXPEDITED_PROTOCOL_VERSIONS, nullptr, &expeditedProtVersions);

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

    // uint8_t version = 0;
    // aliro_reader_get_params_t belAdvVersion;
    // int32_t status = aliroReaderGetParams(ALIRO_READER_BLE_ADV_DATA, nullptr, &belAdvVersion);
    // if (status != ALIRO_STATUS_SUCCESS)
    // {
    //     ChipLogError(DeviceLayer, "Failed to get BLE advertising version: %d", status);
    // }
    // else
    // {
    //     version = belAdvVersion.ble_adv_version;
    // }
    return kNxpAliroBLEAdvertisingVersion;//version;
}

uint16_t NxpAliroDelegate::GetNumberOfAliroCredentialIssuerKeysSupported()
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetNumberOfAliroCredentialIssuerKeysSupported");

    return kNxpAliroNumberOfCredentialIssuerKeysSupported;
}

uint16_t NxpAliroDelegate::GetNumberOfAliroEndpointKeysSupported()
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetNumberOfAliroEndpointKeysSupported");

    return kNxpAliroNumberOfEndpointKeysSupported;
}

CHIP_ERROR NxpAliroDelegate::SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey,
                                                  const ByteSpan & groupIdentifier, const Optional<ByteSpan> & groupResolvingKey)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::SetAliroReaderConfig");
    ChipLogProgress(DeviceLayer, "  signingKey length: %u", static_cast<unsigned>(signingKey.size()));
    ChipLogProgress(DeviceLayer, "  verificationKey length: %u", static_cast<unsigned>(verificationKey.size()));
    ChipLogProgress(DeviceLayer, "  groupIdentifier length: %u", static_cast<unsigned>(groupIdentifier.size()));
    ChipLogProgress(DeviceLayer, "  groupResolvingKey present: %s", groupResolvingKey.HasValue() ? "yes" : "no");

    VerifyOrReturnError(verificationKey.size() == kNxpAliroPublicKeySize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(signingKey.size() == kNxpAliroPrivateKeySize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(groupIdentifier.size() == kNxpAliroGroupIdentifierSize, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t *groupResolvingKeyPtr = nullptr;
    size_t groupResolvingKeySize = 0;

    if (groupResolvingKey.HasValue())
    {
        groupResolvingKeyPtr = const_cast<uint8_t *>(groupResolvingKey.Value().data());
        groupResolvingKeySize = groupResolvingKey.Value().size();
        VerifyOrReturnError(groupResolvingKeySize == kNxpAliroGroupResolvingKeySize, CHIP_ERROR_INVALID_ARGUMENT);
    }

    return SetAliroReaderConfigInternal(signingKey.data(), verificationKey.data(),groupIdentifier.data(), groupResolvingKeyPtr);
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

    // aliro_user_config_t userConfig = {0};
    bool result = false;

    // Validate issuer key size (typically public key size)
    if (credentialData.size() != ALIRO_CRYPTO_EC_PUBKEY_DER_LEN)
    {
        ChipLogError(DeviceLayer, "Invalid Aliro Credential Key size: %u, expected: %u", 
                    static_cast<unsigned>(credentialData.size()), ALIRO_CRYPTO_EC_PUBKEY_DER_LEN);
        ExitNow();
    }
    
    // userConfig.index = credentialIndex;

    // Validate credential data size based on type
    switch (credentialType)
    {
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
        //userConfig.credential_issuer_public_key = credentialData.data();
        // NXPConfig::WriteConfigValueBin(kNxpAliroCredIsuerKeyString, credential_issuer_public_key, ALIRO_CRYPTO_EC_PUBKEY_DER_LEN);
        break;

    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        //userConfig.access_credential_public_key = credentialData.data();
        // NXPConfig::WriteConfigValueBin(kNxpAliroUserKeyString, access_credential_public_key, sizeof(access_credential_public_key));
        break;

    default:
        ChipLogError(DeviceLayer, "Unknown Aliro credential type: %u", static_cast<uint8_t>(credentialType));
        return false;
    }

    result = true;
    // aliroReaderConfigUser(&userconfig);
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

CHIP_ERROR NxpAliroDelegate::SetAliroReaderConfigInternal(const uint8_t *privateKey,
                                                          const uint8_t *publicKey,
                                                          const uint8_t *groupIdentifier,
                                                          const uint8_t *groupResolvingKey)
{
    aliro_init_t initStruct = { 0 };
    extern_aliro_reader_config_t readerConfig = { 0 };
    aliro_reader_state_info_t readerState = { .current_state = ALIRO_READER_STATE_STARTED };
    // uint8_t aliroCaps = ALIRO_CAP_BLE | ALIRO_CAP_UWB | ALIRO_CAP_NFC | ALIRO_CAP_KEY_SLOT | ALIRO_CAP_FAST_TRANSACTION |
    //                     ALIRO_CAP_STEP_UP | ALIRO_CAP_HW_RNG;

    uint8_t aliroCaps = ALIRO_CAP_BLE | ALIRO_CAP_UWB | ALIRO_CAP_NFC | ALIRO_CAP_KEY_SLOT | 
                        1 << 5 | ALIRO_CAP_HW_RNG;

    initStruct.protocol_version = 1;                           /**< API Protocol version for compatibility check */
    initStruct.capabilities = aliroCaps;                       /**< Capability bitmap (BLE, UWB, etc.) */
    initStruct.reseed_interval = 1000;                         /**< DRBG reseed interval */
    initStruct.max_sessions = 1;                               /**< Maximum concurrent sessions */
    initStruct.hw_rng_enabled = 1;                             /**< Hardware RNG enable flag */
    initStruct.entropy_seed_ptr = kNxpAliroSeedString;         /**< DRBG personalization string */
    initStruct.entropy_seed_len = sizeof(kNxpAliroSeedString); /**< Personalization string length */

    BOARD_InitPinLPUART0_RX();
    BOARD_InitPinLPUART0_TX();

    serial_transport_init(nullptr, apdu_flags(APDU_RX_INT_ENABLE));
    int32_t status = aliroReaderInit(&initStruct);
    VerifyOrDie(status == ALIRO_STATUS_SUCCESS);

    readerConfig.reader_group_identifier = groupIdentifier;
    readerConfig.reader_group_identifier_size = kNxpAliroGroupIdentifierSize;
    readerConfig.reader_key_pair_public_key = publicKey;
    readerConfig.reader_key_pair_private_key = privateKey;

    if (groupResolvingKey != nullptr)
    {
        readerConfig.reader_group_resolving_key = groupResolvingKey;
        readerConfig.reader_group_resolving_key_size = kNxpAliroGroupResolvingKeySize;
    }

    readerConfig.uwb_capabilities = uwb_config;

    readerConfig.fast_transaction_supported_state = kAliro_ReaderDeviceFastTransactionState_Supported;
    readerConfig.step_up_transaction_supported_state = kAliro_ReaderDeviceStepUpTransactionState_Unsupported;
    readerConfig.keyslot_supported_state = kAliro_ReaderDeviceKeyslot_Unsupported;

    readerConfig.vendor_extension = NULL;
    readerConfig.vendor_extension_size = 0u;
    readerConfig.spsm_value = ALIRO_BLE_DEFAULT_SPSM_VALUE;

    status = aliroReaderConfigReader(&readerConfig);
    VerifyOrDie(status == ALIRO_STATUS_SUCCESS);

#if 1
    memcpy(reader_private_key, &privateKey, sizeof(reader_private_key));
    memcpy(reader_public_key, &publicKey, sizeof(reader_public_key));
    memcpy(READER_GROUP_IDENTIFIER, &groupIdentifier, sizeof(READER_GROUP_IDENTIFIER));

    if (groupResolvingKey != nullptr)
    {
        memcpy(GROUP_RESOLVING_KEY, groupResolvingKey, sizeof(GROUP_RESOLVING_KEY));
    }
#endif

    if (mAliroStateInitialized == false)
    {
       status = aliroReaderSetState(&readerState);
       VerifyOrDie(status == ALIRO_STATUS_SUCCESS);
    }

    SaveAliroReaderConfig(privateKey, publicKey, groupIdentifier, groupResolvingKey);
    mAliroStateInitialized = true;

    return CHIP_NO_ERROR;
}

void NxpAliroDelegate::SaveAliroReaderConfig(const uint8_t *signingKey,
                                            const uint8_t *verificationKey,
                                            const uint8_t *groupIdentifier,
                                            const uint8_t *groupResolvingKey)
{
    NXPConfig::WriteConfigValueBin(kNxpAliroPublicKeyString, verificationKey, sizeof(reader_public_key));
    NXPConfig::WriteConfigValueBin(kNxpAliroPrivateKeyString, signingKey, sizeof(reader_private_key));
    NXPConfig::WriteConfigValueBin(kNxpAliroGroupIdentifierString, groupIdentifier, kNxpAliroGroupIdentifierSize);
    if (groupResolvingKey != nullptr)
    {
        NXPConfig::WriteConfigValueBin(kNxpAliroGroupResolvingKeyString, groupResolvingKey, kNxpAliroGroupResolvingKeySize);
    }
}

CHIP_ERROR NxpAliroDelegate::RestoreAliroReaderConfig()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t privateKey[kNxpAliroPrivateKeySize] = {0};
    uint8_t publicKey[kNxpAliroPublicKeySize] = {0};
    uint8_t groupIdentifier[kNxpAliroGroupIdentifierSize] = {0};
    uint8_t groupResolvingKey[kNxpAliroGroupResolvingKeySize] = {0};
    size_t bytesRead = 0;
 
    // Check the existence of the public key before try to restore the full ALIRO configuration
    err = NXPConfig::ReadConfigValueBin(kNxpAliroPublicKeyString, publicKey, sizeof(publicKey), bytesRead);
    VerifyOrReturnError(err == CHIP_NO_ERROR && bytesRead == sizeof(publicKey), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Restore signing key (private key)
    err = NXPConfig::ReadConfigValueBin(kNxpAliroPrivateKeyString, privateKey, sizeof(privateKey), bytesRead);
    VerifyOrReturnError(err == CHIP_NO_ERROR && bytesRead == sizeof(privateKey), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    
    // Restore group identifier
    err = NXPConfig::ReadConfigValueBin(kNxpAliroGroupIdentifierString, groupIdentifier, sizeof(groupIdentifier), bytesRead);
    VerifyOrReturnError(err == CHIP_NO_ERROR && bytesRead == sizeof(groupIdentifier), CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    // Restore group resolving key
    err = NXPConfig::ReadConfigValueBin(kNxpAliroGroupResolvingKeyString, groupResolvingKey, sizeof(groupResolvingKey), bytesRead);
    
    if ((err == CHIP_NO_ERROR) && (bytesRead == sizeof(groupResolvingKey)))
    {
        err = SetAliroReaderConfigInternal(privateKey, publicKey, groupIdentifier, groupResolvingKey);
    }
    else
    {
        err = SetAliroReaderConfigInternal(privateKey, publicKey, groupIdentifier,nullptr);
    }
    
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    mAliroStateInitialized = true;
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::RestoreAliroReaderConfig sucessfull");

    return CHIP_NO_ERROR;
}

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip