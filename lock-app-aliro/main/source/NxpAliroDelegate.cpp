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

extern "C" void BOARD_InitPinLPUART0_RX(void);
extern "C" void BOARD_InitPinLPUART0_TX(void);

using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

#define SYNC_CODE_BITMASK(val) (0x01U << ((val)-1U))

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
    // .number_responders_nodes = 1,
    .hopping_configuration_bitmask = 0x50,
    .slot_bitmask                  = 0x02 | 0x04,
    .number_slots_per_round        = 24,
    .mac_mode_bitmask              = 0x40 | 0x01
};

uint8_t GROUP_RESOLVING_KEY[16] = {0};

NxpAliroDelegate NxpAliroDelegate::instance;

NxpAliroDelegate & NxpAliroDelegate::Instance()
{
    return instance;
}

CHIP_ERROR NxpAliroDelegate::GetAliroReaderVerificationKey(MutableByteSpan & verificationKey)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroReaderVerificationKey");
    // The input is not used in this case
    aliro_reader_get_params_t getInput = {0};
    aliro_reader_get_params_rsp_t getResponse;

    CheckAndInitTransportLayer();

    if (!mAliroStateInitialized)
    {
        // Set to size 0 to indicate no value, when the state will be initialized the transport will also be ready.
        verificationKey.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    int32_t status = aliroReaderGetParams(ALIRO_READER_PUBLIC_KEY, &getInput, &getResponse);
    HandleInterfaceStatus(status);

    return chip::CopySpanToMutableSpan(ByteSpan(getResponse.reader_public_key.key), verificationKey);
}

CHIP_ERROR NxpAliroDelegate::GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroReaderGroupIdentifier");
    // The input is not used in this case
    aliro_reader_get_params_t getInput = {0};
    aliro_reader_get_params_rsp_t getResponse;

    CheckAndInitTransportLayer();

    if (!mAliroStateInitialized)
    {
        // Set to size 0 to indicate no value, when the state will be initialized the transport will also be ready.
        groupIdentifier.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    int32_t status = aliroReaderGetParams(ALIRO_READER_GROUP_IDENTIFIER, &getInput, &getResponse);
    HandleInterfaceStatus(status);

    return CopySpanToMutableSpan(ByteSpan(getResponse.reader_group_id.reader_group_identifier), groupIdentifier);
    // return CopySpanToMutableSpan(ByteSpan(READER_GROUP_IDENTIFIER), groupIdentifier);
}

CHIP_ERROR NxpAliroDelegate::GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroReaderGroupSubIdentifier");
    // The input is not used in this case
    aliro_reader_get_params_t getInput = {0};
    aliro_reader_get_params_rsp_t getResponse;

    CheckAndInitTransportLayer();

    int32_t status = aliroReaderGetParams(ALIRO_READER_GROUP_SUB_IDENTIFIER, &getInput, &getResponse);
    HandleInterfaceStatus(status);
    return CopySpanToMutableSpan(ByteSpan(getResponse.reader_group_sub_id.reader_group_sub_identifier), groupSubIdentifier);
}

CHIP_ERROR NxpAliroDelegate::GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroGroupResolvingKey");
    // The input is not used in this case
    aliro_reader_get_params_t getInput = {0};
    aliro_reader_get_params_rsp_t getResponse;

    CheckAndInitTransportLayer();

    if (!mAliroStateInitialized)
    {
        // Set to size 0 to indicate no value, when the state will be initialized the transport will also be ready
        groupResolvingKey.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    int32_t status = aliroReaderGetParams(ALIRO_READER_GROUP_RESOLVING_KEY, &getInput, &getResponse);
    HandleInterfaceStatus(status);
    return CopySpanToMutableSpan(ByteSpan(getResponse.reader_group_resolving_key.key), groupResolvingKey);
 }

CHIP_ERROR NxpAliroDelegate::GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
                                                                                         MutableByteSpan & protocolVersion)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex, index: %u", 
                    static_cast<unsigned>(index));
    // The input is not used in this case
    aliro_reader_get_params_t getInput = {0};
    aliro_reader_get_params_rsp_t getResponse;

    CheckAndInitTransportLayer();

    if (mExpeditedVersionCache.nbOfEntries == 0)
    {
        int32_t status = aliroReaderGetParams(ALIRO_READER_EXPEDITED_PROTOCOL_VERSIONS, &getInput, &getResponse);
        HandleInterfaceStatus(status);

        mExpeditedVersionCache.nbOfEntries = getResponse.expedited_versions.version_count;
        memcpy(mExpeditedVersionCache.versionEntry, getResponse.expedited_versions.versions, 
               mExpeditedVersionCache.nbOfEntries * sizeof(mExpeditedVersionCache.versionEntry[0]));
    }
    // mExpeditedVersionCache.nbOfEntries is at least 1 
    if (index > mExpeditedVersionCache.nbOfEntries - 1)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return CopyProtocolVersionIntoSpan(mExpeditedVersionCache.versionEntry[index], protocolVersion);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR NxpAliroDelegate::GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion)
{
     ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroSupportedBLEUWBProtocolVersionAtIndex, index: %u", 
                    static_cast<unsigned>(index));
    // The input is not used in this case
    aliro_reader_get_params_t getInput = {0};
    aliro_reader_get_params_rsp_t getResponse;

    CheckAndInitTransportLayer();

    if (mBleUwbVersionCache.nbOfEntries == 0)
    {
        int32_t status = aliroReaderGetParams(ALIRO_READER_BLEUWB_PROTOCOL_VERSION, &getInput, &getResponse);
        HandleInterfaceStatus(status);

        mBleUwbVersionCache.nbOfEntries = getResponse.bleuwb_version.version_count;
        memcpy(mBleUwbVersionCache.versionEntry, getResponse.bleuwb_version.versions, 
               mBleUwbVersionCache.nbOfEntries * sizeof(mBleUwbVersionCache.versionEntry[0]));
    }
    // mExpeditedVersionCache.nbOfEntries is at least 1 
    if (index > mBleUwbVersionCache.nbOfEntries - 1)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return CopyProtocolVersionIntoSpan(mBleUwbVersionCache.versionEntry[index], protocolVersion);
}

uint8_t NxpAliroDelegate::GetAliroBLEAdvertisingVersion()
{
    // TODO : Implement retrieval of BLE advertising version from reader
    // ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetAliroBLEAdvertisingVersion");
    // // The input is not used in this case
    // aliro_reader_get_params_t getInput = {0};
    // aliro_reader_get_params_rsp_t getResponse;

    // CheckAndInitTransportLayer();

    // int32_t status = aliroReaderGetParams(ALIRO_READER_BLE_ADV_VERSION, &getInput, &getResponse);
    // HandleInterfaceStatus(status);

    // return getResponse.ble_advertising_version.version;
    return kNxpAliroBLEAdvertisingVersion;
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
    aliro_reader_state_info_t readerState = { .current_state = ALIRO_READER_STATE_STOPPED };

    if (mAliroStateInitialized)
    {
        // Clear all Aliro configuration keys from persistent storage
        NXPConfig::ClearConfigValue(kNxpAliroPublicKeyString);
        NXPConfig::ClearConfigValue(kNxpAliroPrivateKeyString);
        NXPConfig::ClearConfigValue(kNxpAliroGroupIdentifierString);
        NXPConfig::ClearConfigValue(kNxpAliroGroupResolvingKeyString);

        ClearAliroUserConfig();

        mBleUwbVersionCache.clear();
        mExpeditedVersionCache.clear();

        mAliroStateInitialized = false;
        int32_t status = aliroReaderSetState(&readerState);
        HandleInterfaceStatus(status);
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

    int32_t status                 = 0;
    bool result                    = false;
    aliro_user_config_t userConfig = {.access_credential_key_index = (uint8_t)credentialIndex};

    char credentialKeyString[kNxpAliroUserKeyStringSize] = {0};

    CheckAndInitTransportLayer();

    // Validate public key size
    if (credentialData.size() != ALIRO_CRYPTO_EC_PUBKEY_DER_LEN)
    {
        ChipLogError(DeviceLayer, "Invalid Aliro Credential Key size: %u, expected: %u", 
                    static_cast<unsigned>(credentialData.size()), ALIRO_CRYPTO_EC_PUBKEY_DER_LEN);
        ExitNow();
    }

    switch (credentialType)
    {
        case CredentialTypeEnum::kAliroCredentialIssuerKey:
            userConfig.access_credential_public_key = nullptr;
            userConfig.credential_issuer_public_key = credentialData.data();
            snprintf(credentialKeyString, sizeof(credentialKeyString), "%s%u", kNxpAliroCredIsuerKeyString, credentialIndex);
            break;

        case CredentialTypeEnum::kAliroEvictableEndpointKey:
        case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
            userConfig.credential_issuer_public_key = nullptr;
            userConfig.access_credential_public_key = credentialData.data();
            snprintf(credentialKeyString, sizeof(credentialKeyString), "%s%u", kNxpAliroUserKeyString, credentialIndex);
            break;

        default:
            ChipLogError(DeviceLayer, "Unknown Aliro credential type: %u", static_cast<uint8_t>(credentialType));
            return false;
    }

    status = aliroReaderConfigUser(&userConfig);
    HandleInterfaceStatus(status);

    // Save to NVM after the credential was configured in the ALIRO stack
    NXPConfig::WriteConfigValueBin(credentialKeyString, credentialData.data(), credentialData.size());

    result = true;
    ChipLogProgress(DeviceLayer, "Aliro credential stored successfully");

exit:
    return result;
}

bool NxpAliroDelegate::IsDoorUnlocked()
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::GetDoorLockState");

    int32_t status = 0;
    aliro_reader_state_info_t readerState = {0};

    CheckAndInitTransportLayer();

    status = aliroReaderGetState(&readerState);
    HandleInterfaceStatus(status);

    return (readerState.current_state == ALIRO_READER_UNSECURED_STATE);
}

void NxpAliroDelegate::SetDoorLockState(bool locked)
{
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::SetDoorLockState locked=%d", locked);
    
    if (mAliroStateInitialized)
    {
        aliro_reader_state_info_t readerState = {
            .current_state = locked ? ALIRO_READER_SECURED_STATE : ALIRO_READER_UNSECURED_STATE
        };

        CheckAndInitTransportLayer();

        int32_t status = aliroReaderSetState(&readerState);
        HandleInterfaceStatus(status);
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

    // Reuse publicKey to optimize stack usage
    err = RestoreAliroUserConfig(publicKey);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    mAliroStateInitialized = true;
    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::RestoreAliroReaderConfig sucessfull");

    return CHIP_NO_ERROR;
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
    int32_t status;

    aliro_init_t initStruct = { 0 };
    extern_aliro_reader_config_t readerConfig = { 0 };
    aliro_reader_state_info_t readerState = { .current_state = ALIRO_READER_STATE_STARTED };
    //aliro_callback_config_t callbackConfig = { 0 };

    CheckAndInitTransportLayer();

    initStruct.protocol_version = 1u;                          /**< API Protocol version for compatibility check */
    initStruct.capabilities = kNxpAliroCapabilities;           /**< Capability bitmap (BLE, UWB, etc.) */
    initStruct.apdu_chain_buf_size = 512u;
    initStruct.apdu_chain_max_segments = 4u;
    initStruct.main_buf_size = 1024u;
    initStruct.recv_buf_size = 512u;
    initStruct.reseed_interval = 1000u;                        /**< DRBG reseed interval */
    initStruct.max_sessions = 8u;                              /**< Maximum concurrent sessions */
    initStruct.hw_rng_enabled = 1u;                            /**< Hardware RNG enable flag */
    initStruct.entropy_seed_ptr = kNxpAliroSeedString;         /**< DRBG personalization string */
    initStruct.entropy_seed_len = sizeof(kNxpAliroSeedString); /**< Personalization string length */

    status = aliroReaderInit(&initStruct);
    HandleInterfaceStatus(status);

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
    HandleInterfaceStatus(status);

    // callbackConfig.ins = ALIRO_CALLBACK_AUTH_SUCCESS;
    // callbackConfig.is_extended = false;
    // callbackConfig.func = AliroReaderCallback;
    
    // status = aliroReaderRegisterCallback(&callbackConfig);
    // HandleInterfaceStatus();

    status = aliroReaderSetState(&readerState);
    HandleInterfaceStatus(status);
 
    SaveAliroReaderConfig(privateKey, publicKey, groupIdentifier, groupResolvingKey);
    mAliroStateInitialized = true;

    return CHIP_NO_ERROR;
}

void NxpAliroDelegate::SaveAliroReaderConfig(const uint8_t *signingKey,
                                            const uint8_t *verificationKey,
                                            const uint8_t *groupIdentifier,
                                            const uint8_t *groupResolvingKey)
{
    NXPConfig::WriteConfigValueBin(kNxpAliroPublicKeyString, verificationKey, kNxpAliroPublicKeySize);
    NXPConfig::WriteConfigValueBin(kNxpAliroPrivateKeyString, signingKey, kNxpAliroPrivateKeySize);
    NXPConfig::WriteConfigValueBin(kNxpAliroGroupIdentifierString, groupIdentifier, kNxpAliroGroupIdentifierSize);
    if (groupResolvingKey != nullptr)
    {
        NXPConfig::WriteConfigValueBin(kNxpAliroGroupResolvingKeyString, groupResolvingKey, kNxpAliroGroupResolvingKeySize);
    }
}

CHIP_ERROR NxpAliroDelegate::RestoreAliroUserConfig(uint8_t *publicKey)
{
    CHIP_ERROR err                                       = CHIP_NO_ERROR;
    int32_t status                                       = 0;
    size_t bytesRead                                     = 0;
    char credentialKeyString[kNxpAliroUserKeyStringSize] = {0};
    aliro_user_config_t userConfig                       = {0};

    // Restore crendial issuer public keys first, index 1 is the first one configured
    for (uint32_t i = 1; i < kNxpAliroNumberOfCredentialIssuerKeysSupported; i++)
    {
        snprintf(credentialKeyString, sizeof(credentialKeyString), "%s%lu", kNxpAliroCredIsuerKeyString, i);
        err = NXPConfig::ReadConfigValueBin(credentialKeyString, publicKey, kNxpAliroPublicKeySize, bytesRead);
        if (err == CHIP_NO_ERROR && bytesRead == kNxpAliroPublicKeySize)
        {
            userConfig.access_credential_key_index = i;
            userConfig.credential_issuer_public_key = publicKey;
            status = aliroReaderConfigUser(&userConfig);
            HandleInterfaceStatus(status);
        }
    }

    userConfig.credential_issuer_public_key = nullptr;

    // Restore access credential public key next, index 1 is the first one configured
    for (uint32_t i = 1; i < kNxpAliroNumberOfEndpointKeysSupported; i++)
    {
        snprintf(credentialKeyString, sizeof(credentialKeyString), "%s%lu", kNxpAliroUserKeyString, i);
        err = NXPConfig::ReadConfigValueBin(credentialKeyString, publicKey, kNxpAliroPublicKeySize, bytesRead);
        if (err == CHIP_NO_ERROR && bytesRead == kNxpAliroPublicKeySize)
        {
            userConfig.access_credential_key_index = i;
            userConfig.access_credential_public_key = publicKey;
            status = aliroReaderConfigUser(&userConfig);
            HandleInterfaceStatus(status);
        }
    }

    return err;
}
CHIP_ERROR NxpAliroDelegate::ClearAliroUserConfig()
{
    char credentialKeyString[kNxpAliroUserKeyStringSize] = {0};

    // Clear credential issuer public keys first
    for (uint32_t i = 1; i < kNxpAliroNumberOfCredentialIssuerKeysSupported; i++)
    {
        snprintf(credentialKeyString, sizeof(credentialKeyString), "%s%lu", kNxpAliroCredIsuerKeyString, i);
        NXPConfig::ClearConfigValue(credentialKeyString);
    }

    // Clear access credential public keys next
    for (uint32_t i = 1; i < kNxpAliroNumberOfEndpointKeysSupported; i++)
    {
        snprintf(credentialKeyString, sizeof(credentialKeyString), "%s%lu", kNxpAliroUserKeyString, i);
        NXPConfig::ClearConfigValue(credentialKeyString);
    }

    ChipLogProgress(DeviceLayer, "NxpAliroDelegate::ClearAliroUserConfig successful");

    return CHIP_NO_ERROR;
}

void NxpAliroDelegate::CheckAndInitTransportLayer()
{
    if(!mAliroTransportInitialized)
    {
        BOARD_InitPinLPUART0_RX();
        BOARD_InitPinLPUART0_TX();

        serial_transport_init(NULL, apdu_flags(APDU_RX_INT_ENABLE));

        mAliroTransportInitialized = true;
    }
}
void NxpAliroDelegate::AliroReaderCallback(uint8_t ins, void* param)
{

}

void NxpAliroDelegate::HandleInterfaceStatus(int32_t status)
{
    if (status != 0)
    {
           ChipLogError(DeviceLayer, "NxpAliroDelegate::HandleInterfaceStatus - communication error");
        // Define what happens when there's an interface error.
        chipDie();
    }
}

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip