/*! *********************************************************************************
* \addtogroup NXP BLE CS Localization functions
*
********************************************************************************** */
/*! *********************************************************************************
* \file nxp_cs_app_localization.c
*
* Copyright 2025 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

#include "fsl_component_mem_manager.h"

#include "ranging_client_interface.h"

#include "app_localization.h"
#include "ble_general.h"
#include "ble_service_discovery.h"
#include "ble_sig_defines.h"
#include "channel_sounding.h"
#include "gap_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "ble_utils.h"
#include "gatt_types.h"
#include "ble_constants.h"
#include "app_localization_algo.h"

typedef enum appEvent_tag
{
    mAppEvt_EncryptionChanged_c,
    mAppEvt_ExchangeMtuComplete_c,
    mAppEvt_ServiceDiscoveryComplete_c,
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_WriteCharacteristicDescriptorComplete_c,
    mAppEvt_ReadCharacteristicValueComplete_c,
    mAppEvt_WriteCharacteristicValueComplete_c,
    mAppEvt_GattProcError_c,
    mAppEvt_PeerDisconnected_c
} appEvent_t;

typedef enum appState_tag
{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppEncryptLink_c,
    mAppLocalizationSetup_c,
    mAppRunning_c
} appState_t;

typedef struct appPeerInfo_tag
{
    deviceId_t          deviceId;
    bool_t              isBonded;
    uint8_t             nvmIndex;
    bool_t              isSubscribed;
    appState_t          appState;
    rasStaticConfig_t   rasConfigInfo;
}appPeerInfo_t;

static appPeerInfo_t maPeerInformation[gAppMaxConnections_c];

/* Buffer used for Characteristic related procedures */
static gattAttribute_t      *mpCharProcBuffer = NULL;
static gattCharacteristic_t  mpRasCharacteristic;

uint16_t gFilterShellVal = (uint16_t)gNoFilter_c;

static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_GattClientCallback(deviceId_t serverDeviceId,
                                    gattProcedureType_t procedureType,
                                    gattProcedureResult_t procedureResult,
                                    bleResult_t error);
static void BleApp_GattIndicationCallback(deviceId_t serverDeviceId,
                                    uint16_t characteristicValueHandle,
                                    uint8_t* aValue,
                                    uint16_t valueLength);
static void BleApp_StoreServiceHandles(deviceId_t peerDeviceId,
                                    gattService_t *pService);
static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId,
                                    servDiscEvent_t* pEvent);

static bleResult_t BleApp_ConfigureRasServer
(
    deviceId_t peerDeviceId,
    uint16_t value,
    uint16_t handle
);

static void BleApp_HandleRasSubscription
(
    deviceId_t peerDeviceId
);

void app_ble_init_callback(void)
{
    for (uint8_t id = 0; id < (uint8_t)gAppMaxConnections_c; id++)
    {
        maPeerInformation[id].deviceId = gInvalidDeviceId_c;
        maPeerInformation[id].appState = mAppIdle_c;
    }
}

void app_connection_callback(deviceId_t id, gapConnectionEvent_t* event)
{
    switch(event->eventType)
    {
        //case gConnEvtPairingComplete_c:
        case gConnEvtEncryptionChanged_c:
        {
            appLocalization_rangeCfg_t locConfig;

            maPeerInformation[id].deviceId = id;
            maPeerInformation[id].isBonded = FALSE;
            maPeerInformation[id].nvmIndex = gInvalidNvmIndex_c;

            (void)Gap_CheckIfBonded(id, &maPeerInformation[id].isBonded, &maPeerInformation[id].nvmIndex);

            /* Read current CS config and update procedure repeat interval */
            (void)AppLocalization_ReadConfig(id, &locConfig);

            /* Estimated algo durations (4 AP, 80 channels) for procedure repeat frequency calculation */
            uint32_t algoDurationMs = 0U;
#if defined(gAppUseRADEAlgorithm_d) && (gAppUseRADEAlgorithm_d == 1)
            algoDurationMs += 45U;
#endif /* gAppUseRADEAlgorithm_d */
#if defined(gAppUseCDEAlgorithm_d) && (gAppUseCDEAlgorithm_d == 1)
            algoDurationMs += 20U;
#endif /* gAppUseCDEAlgorithm_d */

            uint32_t procInterval = CS_PROC_DURATION_MS_MAX + POSTPROC_VERB_DURATION_MS_MIN + APPLICATION_OFFSET_DURATION_MS + algoDurationMs;
#if defined (BOARD_LOCALIZATION_REVISION_SUPPORT) && (BOARD_LOCALIZATION_REVISION_SUPPORT == 1U)
            procInterval +=  LOC_BOARD_PROC_REPEAT_DELAY;
#endif
            /* Convert ms to connection intervals */
            procInterval = 1U + (procInterval * 1000U)/(((uint32_t)(event->eventData.connectedEvent.connParameters.connInterval)) * 1250U);
            locConfig.minPeriodBetweenProcedures = (uint16_t)procInterval;
            locConfig.maxPeriodBetweenProcedures = (uint16_t)procInterval;

            (void)AppLocalization_WriteConfig(id, &locConfig);
#if defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1)
            AppLocalization_TimeInfoSetConnInterval(event->eventData.connectedEvent.connParameters.connInterval);
#endif /* defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1) */

            BleApp_StateMachineHandler(id, mAppEvt_EncryptionChanged_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Make sure it is not a Matter Commissioning Connection*/
            if (maPeerInformation[id].appState != mAppIdle_c)
            {
                BleApp_StateMachineHandler(id, mAppEvt_PeerDisconnected_c);
            }
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

void app_gap_callback(gapGenericEvent_t* event)
{
    bleResult_t status = gBleSuccess_c;

    switch(event->eventType)
    {
        case gInitializationComplete_c:
        {
            status = GattClient_RegisterProcedureCallback(BleApp_GattClientCallback);
            if (status != gBleSuccess_c)
            {
                // PRINTF("Error in GattClient_RegisterProcedureCallback");
                break;
            }

            status = GattClient_RegisterIndicationCallback(BleApp_GattIndicationCallback);
            if (status != gBleSuccess_c)
            {
                // PRINTF("Error in GattClient_RegisterIndicationCallback");
                break;
            }

            BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

            status = AppLocalization_HostInitHandler();
            if (status != gBleSuccess_c)
            {
                // PRINTF("Error in AppLocalization_HostInitHandler");
                break;
            }
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

void app_gatt_callback(deviceId_t id, gattServerEvent_t* event)
{
    /*Demo Gatt application callback*/
}

static void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    appEvent_t event
)
{
    if (maPeerInformation[peerDeviceId].deviceId != gInvalidDeviceId_c)
    {
        switch (maPeerInformation[peerDeviceId].appState)
        {
        case mAppIdle_c:
        {
            if (event == mAppEvt_EncryptionChanged_c)
            {
                // PRINTF("Link encrypted");
                /* Moving to Exchange MTU State */
                (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
                maPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            if (event == mAppEvt_ExchangeMtuComplete_c)
            {
                /* Start service discovery for Ranging Service */
                bleUuid_t rasUuid;
                /* Moving to Localization Setup State*/
                // PRINTF("MTU Exchange complete");
                maPeerInformation[peerDeviceId].appState = mAppLocalizationSetup_c;
                rasUuid.uuid16 = gBleSig_RangingService_d;
                (void)BleServDisc_FindService(peerDeviceId, gBleUuidType16_c, &rasUuid);
            }
            else
            {
                if (event == mAppEvt_GattProcError_c)
                {
                    (void)Gap_Disconnect(peerDeviceId);
                }
            }
        }
        break;

        case mAppLocalizationSetup_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Read RAS features characteristic */
                mpRasCharacteristic.value.handle = maPeerInformation[peerDeviceId].rasConfigInfo.featuresHandle;
                mpRasCharacteristic.value.uuidType = gBleUuidType16_c;
                mpRasCharacteristic.value.uuid.uuid16 = gBleSig_RasFeature_d;
                mpRasCharacteristic.value.paValue = (uint8_t*)MEM_BufferAlloc(sizeof(uint32_t));
                if (mpRasCharacteristic.value.paValue != NULL)
                {
                    (void)GattClient_ReadCharacteristicValue(peerDeviceId,
                                                            &mpRasCharacteristic,
                                                            (uint16_t)(sizeof(uint32_t)));
                }
            }

            /* Wrote Filter command */
            else if (event == mAppEvt_WriteCharacteristicValueComplete_c)
            {
                if( mpCharProcBuffer == NULL )
                {
                    mpCharProcBuffer = (gattAttribute_t*)MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
                }

                if( mpCharProcBuffer != NULL )
                {
                    BleApp_HandleRasSubscription(peerDeviceId);
                }
            }
            else if (event == mAppEvt_ReadCharacteristicValueComplete_c)
            {
                if (mpRasCharacteristic.value.handle == maPeerInformation[peerDeviceId].rasConfigInfo.featuresHandle)
                {
                    uint32_t rasFeatures = Utils_ExtractFourByteValue(mpRasCharacteristic.value.paValue);
                    RasClient_SetRasSupportedFeatures(peerDeviceId, rasFeatures);

                    if( mpCharProcBuffer == NULL )
                    {
                        mpCharProcBuffer = (gattAttribute_t*)MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
                    }

                    if( mpCharProcBuffer != NULL )
                    {
                        BleApp_HandleRasSubscription(peerDeviceId);
                    }
                }
            }
            else if (event == mAppEvt_WriteCharacteristicDescriptorComplete_c)
            {
                if( mpCharProcBuffer == NULL )
                {
                    mpCharProcBuffer = (gattAttribute_t*)MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
                }

                if( mpCharProcBuffer != NULL )
                {
                    BleApp_HandleRasSubscription(peerDeviceId);
                }
            }
            else if (event == mAppEvt_ServiceDiscoveryFailed_c)
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                /* For MISRA compliance */
            }
        }
        break;

        case mAppRunning_c:
        {
            if (event == mAppEvt_WriteCharacteristicDescriptorComplete_c)
            {
                // PRINTF("Channel Sounding configuration complete");
                (void)MEM_BufferFree(mpCharProcBuffer);
                mpCharProcBuffer = NULL;
            }
            else
            {
                /* ignore other event types */
            }
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
        }
    }
    /* Handle disconnect event in all application states. */
    if (event == mAppEvt_PeerDisconnected_c)
    {
        // PRINTF("Disconnected");
        gFilterShellVal = (uint16_t)gNoFilter_c;
        maPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
        maPeerInformation[peerDeviceId].appState = mAppIdle_c;
        maPeerInformation[peerDeviceId].isSubscribed = FALSE;
        AppLocalization_ResetPeer(peerDeviceId, TRUE, maPeerInformation[peerDeviceId].nvmIndex);
    }
}

static bleResult_t BleApp_ConfigureRasServer
(
    deviceId_t peerDeviceId,
    uint16_t value,
    uint16_t handle
)
{
    bleResult_t result = gBleSuccess_c;

    mpCharProcBuffer->handle = handle;
    mpCharProcBuffer->uuidType = gBleUuidType16_c;
    mpCharProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
    mpCharProcBuffer->valueLength = 1U;

    result = GattClient_WriteCharacteristicDescriptor(peerDeviceId,
                                                      mpCharProcBuffer,
                                                      (uint16_t)sizeof(value),
                                                      (uint8_t*)&value);

    return result;
}

static void BleApp_HandleRasSubscription
(
    deviceId_t peerDeviceId
)
{
    static uint16_t lastWrittenHandle = gGattDbInvalidHandle_d;
    uint16_t value = gCccdIndication_c;
    bleResult_t result = gBleSuccess_c;
    bool_t filteringDone = TRUE;

    if (lastWrittenHandle == gGattDbInvalidHandle_d)
    {
        if (gFilterShellVal != (uint16_t)gNoFilter_c)
        {
            /* Set the filter and proceed as normal afterwards */
            result = RasClient_RasSetFilter(peerDeviceId, gFilterShellVal, TRUE, &filteringDone);
        }

        if ((result == gBleSuccess_c) && (filteringDone == TRUE))
        {
            gFilterShellVal = (uint16_t)gNoFilter_c;

            /* Write the RAS-CP */
            value = gCccdIndication_c;
            lastWrittenHandle = (uint16_t)(maPeerInformation[peerDeviceId].rasConfigInfo.controlPointHandle + 1U);

            result = BleApp_ConfigureRasServer(peerDeviceId, value, lastWrittenHandle);
        }
    }
    else if (lastWrittenHandle == (maPeerInformation[peerDeviceId].rasConfigInfo.controlPointHandle + 1U))
    {
        lastWrittenHandle = (uint16_t)(maPeerInformation[peerDeviceId].rasConfigInfo.dataReadyHandle + 1U);
        result = BleApp_ConfigureRasServer(peerDeviceId, value, lastWrittenHandle);
    }
    else if (lastWrittenHandle == (maPeerInformation[peerDeviceId].rasConfigInfo.dataReadyHandle + 1U))
    {
        lastWrittenHandle = (uint16_t)(maPeerInformation[peerDeviceId].rasConfigInfo.dataOverwrittenHandle + 1U);
        result = BleApp_ConfigureRasServer(peerDeviceId, value, lastWrittenHandle);
    }
    else if (lastWrittenHandle == (maPeerInformation[peerDeviceId].rasConfigInfo.dataOverwrittenHandle + 1U))
    {
        lastWrittenHandle = (uint16_t)(maPeerInformation[peerDeviceId].rasConfigInfo.onDemandDataHandle + 1U);
        result = BleApp_ConfigureRasServer(peerDeviceId, value, lastWrittenHandle);
    }
    else
    {
        /* MISRA compliance */
    }

    if ((result == gBleSuccess_c) && ((lastWrittenHandle ==
        (maPeerInformation[peerDeviceId].rasConfigInfo.onDemandDataHandle + 1U)) ||
        (lastWrittenHandle == (maPeerInformation[peerDeviceId].rasConfigInfo.realTimeDataHandle + 1U))))
    {
         /* Begin localization procedure */
        maPeerInformation[peerDeviceId].appState = mAppRunning_c;
        maPeerInformation[peerDeviceId].isSubscribed = TRUE;
        lastWrittenHandle = gGattDbInvalidHandle_d;

        uint8_t nvmIndex = gInvalidNvmIndex_c;
        bool_t isBonded = FALSE;

        result = Gap_CheckIfBonded(peerDeviceId, &isBonded, &nvmIndex);

        if ((result == gBleSuccess_c) && (nvmIndex != gInvalidNvmIndex_c))
        {
            (void)AppLocalization_WriteCachedRemoteCapabilities(peerDeviceId, nvmIndex);
        }
        
    }
}

static void BleApp_GattClientCallback
(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
)
{
    if (procedureResult == gGattProcError_c)
    {
        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
    }
    else
    {
        if (procedureResult == gGattProcSuccess_c)
        {
            switch(procedureType)
            {
                case gGattProcWriteCharacteristicDescriptor_c:
                {
                    BleApp_StateMachineHandler(serverDeviceId, mAppEvt_WriteCharacteristicDescriptorComplete_c);
                }
                break;

                case gGattProcReadCharacteristicValue_c:
                case gGattProcReadUsingCharacteristicUuid_c:
                {
                    BleApp_StateMachineHandler(serverDeviceId, mAppEvt_ReadCharacteristicValueComplete_c);
                }
                break;

                case gGattProcWriteCharacteristicValue_c:
                {
                    BleApp_StateMachineHandler(serverDeviceId, mAppEvt_WriteCharacteristicValueComplete_c);
                }
                break;

                case gGattProcExchangeMtu_c:
                {
                   BleApp_StateMachineHandler(serverDeviceId, mAppEvt_ExchangeMtuComplete_c);
                }
                break;

                default:
                {
                    ; /* No action required */
                }
                break;
            }
        }
    }

    /* Signal Service Discovery Module */
    BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
}

static void BleApp_GattIndicationCallback
(
    deviceId_t  serverDeviceId,
    uint16_t    characteristicValueHandle,
    uint8_t*    aValue,
    uint16_t    valueLength
)
{
    if (characteristicValueHandle == maPeerInformation[serverDeviceId].rasConfigInfo.controlPointHandle)
    {
        /* Received an indication from peer for the RAS control point characteristic */
        (void)RasClient_ProcessRasCPRsp(serverDeviceId, aValue, valueLength);
    }

    if (characteristicValueHandle == maPeerInformation[serverDeviceId].rasConfigInfo.dataReadyHandle)
    {
        /* Received an indication from peer for the RAS control point characteristic */
        (void)RasClient_ProcessRasDataReadyIndications(serverDeviceId, aValue, valueLength);
    }

    if (characteristicValueHandle == maPeerInformation[serverDeviceId].rasConfigInfo.dataOverwrittenHandle)
    {
        /* Received an indication from peer for the RAS Data Overwritten characteristic */
        (void)RasClient_ProcessRasDataOverwrittenIndications(serverDeviceId, aValue, valueLength);
    }

    if ((characteristicValueHandle ==  maPeerInformation[serverDeviceId].rasConfigInfo.onDemandDataHandle) ||
        (characteristicValueHandle ==  maPeerInformation[serverDeviceId].rasConfigInfo.realTimeDataHandle))
    {
        /* Received localization data from peer */
        (void)RasClient_StorePeerMeasurementData(serverDeviceId, aValue, valueLength);
    }
}

static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t* pEvent)
{
    switch(pEvent->eventType)
    {
        /* Store the discovered handles for later use. */
        case gServiceDiscovered_c:
        {
            BleApp_StoreServiceHandles(peerDeviceId, pEvent->eventData.pService);
        }
        break;

        /* Service discovery has finished, run the state machine. */
        case gDiscoveryFinished_c:
        {
            if (pEvent->eventData.success)
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryComplete_c);
            }
            else
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryFailed_c);
            }
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

static void BleApp_StoreServiceHandles
(
    deviceId_t peerDeviceId,
    gattService_t *pService
)
{
   if ((pService->uuidType == gBleUuidType16_c) &&
       (pService->uuid.uuid16 == gBleSig_RangingService_d))
    {
        /* Found Wireless Ranging Service */
        maPeerInformation[peerDeviceId].rasConfigInfo.serviceHandle = pService->startHandle;

        for (uint8_t i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasControlPoint_d))
            {
                /* Found RAS Control Point Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.controlPointHandle =
                pService->aCharacteristics[i].value.handle;
                /* Register value with localization component */
                RasClient_SetRasControlPointHandle(peerDeviceId,
                                                            pService->aCharacteristics[i].value.handle);
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasProcDataReady_d))
            {
                /* Found RAS Ranging Data Ready Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.dataReadyHandle =
                pService->aCharacteristics[i].value.handle;
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasOnDemandProcData_d))
            {
                /* Found RAS Stored Ranging Data Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.onDemandDataHandle =
                pService->aCharacteristics[i].value.handle;
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasRealTimeProcData_d))
            {
                /* Found RAS Stored Ranging Data Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.realTimeDataHandle =
                pService->aCharacteristics[i].value.handle;
                RasClient_SetRasRealTimeHandle(peerDeviceId,
                                                        pService->aCharacteristics[i].value.handle);
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasprocDataOverwritten_d))
            {
                /* Found RAS Procedure Data Overwritten Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.dataOverwrittenHandle =
                pService->aCharacteristics[i].value.handle;
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasFeature_d))
            {
                /* Found RAS Features Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.featuresHandle =
                pService->aCharacteristics[i].value.handle;
            }
        }
    }
}
