/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    Copyright 2024-2025 NXP
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

#include "BLEApplicationManager.h"

#include <platform/ConfigurationManager.h>
#include <platform/ConnectivityManager.h>
#include <platform/PlatformManager.h>
#include <platform/nxp/common/ble/BLEManagerCommon.h>

extern "C" {
#include "app_localization.h"
}

#include "app_localization_algo.h"
#include "ble_sig_defines.h"
#include "gap_types.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"
#include "EmbeddedTypes.h"

extern "C"
{
void btSettingsInit(void);
void btSettingsWipe(void);

void app_ble_init_callback(void);
void app_connection_callback(deviceId_t id, gapConnectionEvent_t* event);
void app_gap_callback(gapGenericEvent_t* event);
void app_gatt_callback(deviceId_t id, gattServerEvent_t* event);
}

static void BleApp_CsEventHandler(deviceId_t deviceId, void *pData, appCsEventType_t eventType);
static void BleApp_PrintMeasurementResults(deviceId_t deviceId, localizationAlgoResult_t *pResult);

gapAdStructure_t* ble_cs_adv_data_cb(uint8_t *size);

using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::NXP::App;

static uint16_t mProcedureCount = 0x0U;

BLEApplicationManager BLEApplicationManager::sInstance;

void BLEApplicationManager::Init(void)
{
    auto * bleManager = &chip::DeviceLayer::Internal::BLEMgrImpl();
    bleResult_t status = gBleSuccess_c;

    /* Register BLE application callbacks*/
    bleManager->RegisterAppCallbacks(app_ble_init_callback, app_connection_callback, app_gap_callback, app_gatt_callback);

    status = AppLocalization_Init(gCsRoleReflector_c,
                                BleApp_CsEventHandler,
                                BleApp_PrintMeasurementResults);
    if (status != gBleSuccess_c)
    {
        ChipLogError(DeviceLayer, "Error in AppLocalization_Init");
    }

#if gAppUseBonding_d
    btSettingsInit();
#endif
}

void BLEApplicationManager::EnableMultipleConnectionsHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(EnableMultipleConnections, 0);
}

void BLEApplicationManager::EnableMultipleConnections(intptr_t arg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Check the status of the commissioning */
    if (ConfigurationMgr().IsFullyProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Multiple connections, start advertising");
        auto * bleManager = &chip::DeviceLayer::Internal::BLEMgrImpl();
        /* Register BLE arvertising data callback */
        bleManager->RegisterAdvDataCallback(ble_cs_adv_data_cb);
        /* Set BLE service mode */
        bleManager->SetBLEServiceMode(kMultipleBLE_Enabled);
        /* Start advertising */
        err = ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Error during ConnectivityMgr().SetBLEAdvertisingEnabled()");
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Device must be commissioned before adding multiple BLE connections");
    }
}

void BLEApplicationManager::FactoryReset(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = DeviceLayer::Internal::BLEMgrImpl().DisconnectAndUnbond();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during BLEMgrImpl().DisconnectAndUnbond()");
    }

#if gAppUseBonding_d
    btSettingsWipe();
#endif
}

extern "C" bleResult_t App_PostCallbackMessage(void (*handler)(void*), void* param)
{
    BLEManagerCommon::AddBleAppMsgHandler(BLEManagerCommon::BLE_MSG_APP_EV_CB, handler, param);

    return gBleSuccess_c;
}

gapAdStructure_t* ble_cs_adv_data_cb(uint8_t *size)
{
    static uint8_t advDataFlags = (gLeGeneralDiscoverableMode_c | gBrEdrNotSupported_c);
    gapAdStructure_t* adv_data;
    static uint8_t ranging_serv[2] = { UuidArray(gBleSig_RangingService_d) };

    *size = 2;
    adv_data = (gapAdStructure_t*) malloc((*size) * sizeof(gapAdStructure_t));

    adv_data[0].length = *size;
    adv_data[0].adType = gAdFlags_c;
    adv_data[0].aData  = (uint8_t *) (&advDataFlags);

    adv_data[1].length = NumberOfElements(ranging_serv) + 1;
    adv_data[1].adType = gAdIncomplete16bitServiceList_c;
    adv_data[1].aData  = (uint8_t *)ranging_serv;

    return adv_data;
}

static void BleApp_CsEventHandler(deviceId_t deviceId, void *pData, appCsEventType_t eventType)
{
    switch(eventType)
    {
        case gCsSecurityEnabled_c:
        {
            ChipLogProgress(DeviceLayer, "CS security enabled.");

        }
        break;

        case gConfigComplete_c:
        case gLocalConfigWritten_c:
        {
            ChipLogProgress(DeviceLayer, "Localization config complete.");
        }
        break;

        case gDistanceMeastStarted_c:
        {
            mProcedureCount = 0U;
            ChipLogProgress(DeviceLayer, "Distance measurement started.");
        }
        break;

        case gLocalMeasurementComplete_c:
        {
            mProcedureCount++;
            ChipLogProgress(DeviceLayer, "Distance measurement complete. Local data available.");
        }
        break;

        case gDataOverwritten_c:
        {
            ChipLogProgress(DeviceLayer, "Received Data Overwritten Indication! Clearing local data.");
        }
        break;

        case gErrRasOpCodeNotSupported_c:
        {
            ChipLogProgress(DeviceLayer, "RAS error op code not supported!");
        }
        break;

        case gErrRasInvalidParameter_c:
        {
            ChipLogProgress(DeviceLayer, "RAS error invalid parameter!");
        }
        break;

        case gErrRasParameterNotSupported_c:
        {
            ChipLogProgress(DeviceLayer, "RAS error parameter not supported!");
        }
        break;

        case gErrRasAbortUnsuccessful_c:
        {
            ChipLogProgress(DeviceLayer, "RAS error abort unsuccessul!");
        }
        break;

        case gErrRasProcedureNotCompleted_c:
        {
            ChipLogProgress(DeviceLayer, "RAS error procedure not completed!");
        }
        break;

        case gErrRasServerBusy_c:
        {
            ChipLogProgress(DeviceLayer, "RAS error server busy!");
        }
        break;

        case gErrRasNoRecordsFound_c:
        {
            ChipLogProgress(DeviceLayer, "RAS error no records found!");
        }
        break;

        case gErrorEvent_c:
        {
            appLocalizationError_t *pError = (appLocalizationError_t*)pData;

            ChipLogProgress(DeviceLayer, "Error event for deviceId = %d", (uint8_t)deviceId);

            switch (*pError)
            {
                case gAppLclErrorRLSC_c:
                {
                    ChipLogProgress(DeviceLayer, "Error occured! Source: csReadLocalSupportedCapabilities!");
                }
                break;

                case gAppLclUnexpectedCC_c:
                {
                    ChipLogProgress(DeviceLayer, "Received an unexpected Config Complete Event!");
                }
                break;

                case gAppLclUnexpectedRRSCC_c:
                {
                    ChipLogProgress(DeviceLayer, "Received an unexpected Read Remote Supported Capabilities Complete Event!");
                }
                break;

                case gAppLclUnexpectedPEC_c:
                {
                    ChipLogProgress(DeviceLayer, "Received an unexpected Procedure Enable Complete Event!");
                }
                break;

                case gAppLclUnexpectedSRE_c:
                {
                    ChipLogProgress(DeviceLayer, "Received an unexpected Subevent Result Event!");
                }
                break;

                case gAppLclUnexpectedSDS_c:
                {
                    ChipLogProgress(DeviceLayer, "Received an unexpected Set Default Settings Event!");
                }
                break;


                case gAppLclUnexpectedSRCE_c:
                {
                    ChipLogProgress(DeviceLayer, "Received an unexpected Subevent Result Continue Event!");
                }
                break;

                case gAppLclErrorRRSCCC_c:
                {
                    ChipLogProgress(DeviceLayer, "Error occured! Source: readRemoteSupportedCapabilitiesComplete!");
                }
                break;

                case gAppLclErrorSEC_c:
                {
                    ChipLogProgress(DeviceLayer, "Error occured! Source: securityEnableComplete!");
                }
                break;

                case gAppLclInvalidDeviceId_c:
                {
                    ChipLogProgress(DeviceLayer, "Received an invalid device Id!");
                }
                break;

                case gAppLclSDSConfigError_c:
                {
                    ChipLogProgress(DeviceLayer, "CS_SetDefaultSettings command failed!");
                }
                break;

                case gAppLclCCConfigError_c:
                {
                    ChipLogProgress(DeviceLayer, "CS_CreateConfig command failed!");
                }
                break;

                case gAppLclRRSCError_c:
                {
                    ChipLogProgress(DeviceLayer, "Error status received! csReadRemoteSupportedCsCapabilities command status event!");
                }
                break;

                case gAppLclSEError_c:
                {
                    ChipLogProgress(DeviceLayer, "Error status received! csSecurityEnable command status event!");
                }
                break;

                case gAppLclCCError_c:
                {
                    ChipLogProgress(DeviceLayer, "Error status received! csCreateConfig command status event!");
                }
                break;

                case gAppLclAlgoNotRun_c:
                {
                    ChipLogProgress(DeviceLayer, "Algorithm did not run, procedure likely failed on peer.");
                }
                break;

                case gAppLclStartMeasurementFail_c:
                {
                    ChipLogProgress(DeviceLayer, "Start measurement failed!");
                }
                break;

                case gAppLclProcStatusFailed_c:
                {
                    ChipLogProgress(DeviceLayer, "Procedure done status error received!");
                }
                break;

                case gAppLclProcedureAborted_c:
                {
                    ChipLogProgress(DeviceLayer, "All subsequent CS procedures aborted!");
                }
                break;

                case gAppLclRasTransferFailed_c:
                {
                    ChipLogProgress(DeviceLayer, "RAS - Received an error response from RAS server!");
                }
                break;

                case gAppLclInvalidProcCounter_c:
                {
                    ChipLogProgress(DeviceLayer, "RAS - Received an invalid procedure index!");
                }
                break;

		        case gAppLclInvalidProcIndex_c:
                {
                    ChipLogProgress(DeviceLayer, "RAS - Received a data ready indication for a procedure index different from the local one!");
                }
                break;

                case gAppLclInvalidSegmentCounter_c:
                {
                   ChipLogProgress(DeviceLayer, "RAS - Received an invalid segment counter in data notification!");
                }
                break;

                case gAppLclSubeventStatusFailed_c:
                {
                    ChipLogProgress(DeviceLayer, "Subevent status failed!");
                }
                break;

                case gAppLclNoSubeventMemoryAvailable_c:
                {
                    ChipLogProgress(DeviceLayer, "No more memory available for a local subevent!");
                }
                break;

                case gAppLclErrorProcessingSubevent_c:
                {
                    ChipLogProgress(DeviceLayer, "An error occured in the processing of subevent data!");
                }
                break;

                case gAppLclAlgoNotRunNoDataReady_c:
                {
                    ChipLogProgress(DeviceLayer, "Algorithm did not run - No Data Ready from peer!");
                }
                break;

                case gAppLclAlgoNotRunNoRangingData_c:
                {
                    ChipLogProgress(DeviceLayer, "Algorithm did not run - Ranging Data not complete!");
                }
                break;

                case gAppLclAlgoNotRunNoRealTimeData_c:
                {
                    ChipLogProgress(DeviceLayer, "Algorithm did not run - Real Time Ranging Data not complete!");
                    // maPeerInformation[deviceId].isSubscribed = FALSE;
                }
                break;

                default:
                {
                    ChipLogProgress(DeviceLayer, "Unknown error!");
                }
                break;
            }
        }
        break;

        case gErrorSubeventAborted_c:
        {
            uint8_t abortReason = *((uint8_t*)pData);

            ChipLogProgress(DeviceLayer, "Current CS subevent aborted for deviceId = %d", (uint8_t)deviceId);

            switch (abortReason)
            {
                case (uint8_t)gAppLclNoCsSync_c:
                {
                    ChipLogProgress(DeviceLayer, "No CS_SYNC (mode0) received.");
                }
                break;

                case (uint8_t)gAppLclScheduleConflict_c:
                {
                    ChipLogProgress(DeviceLayer, "Scheduling conflicts or limited resources.");
                }
                break;

                case (uint8_t)gAppLclTimePassed_c:
                {
                    ChipLogProgress(DeviceLayer, "Time passed.");
                }
                break;

                case (uint8_t)gAppLclInvalidArguments_c:
                {
                    ChipLogProgress(DeviceLayer, "Invalid arguments.");
                }
                break;

                case (uint8_t)gAppLclAborted_c:
                {
                    ChipLogProgress(DeviceLayer, "Aborted.");
                }
                break;

                case (uint8_t)gAppLclUnspecifiedReasons_c:
                {
                    ChipLogProgress(DeviceLayer, "Unspecified reasons.");
                }
                break;

                default:
                {
                    ChipLogProgress(DeviceLayer, "Unknown!");
                }
                break;
            }
        }
        break;

        case gErrorProcedureAborted_c:
        {
            uint8_t abortReason = *((uint8_t*)pData);

            ChipLogProgress(DeviceLayer, "All subsequent CS procedures aborted for deviceId = %d", (uint8_t)deviceId);

            switch (abortReason)
            {
                case (uint8_t)gAppLclLocalHost_c:
                {
                    ChipLogProgress(DeviceLayer, "Abort because of local Host or remote request.");
                }
                break;

                case (uint8_t)gAppLclRequiredChannelNumber_c:
                {
                    ChipLogProgress(DeviceLayer, "Abort because filtered channel map has less than 15 channels.");
                }
                break;

                case (uint8_t)gAppLclChannelMapInstant_c:
                {
                    ChipLogProgress(DeviceLayer, "Abort because the channel map update instant has passed.");
                }
                break;

                case (uint8_t)gAppLclUnspecifiedReasons_c:
                {
                    ChipLogProgress(DeviceLayer, "Abort because of unspecified reasons.");
                }
                break;

                default:
                {
                    ChipLogProgress(DeviceLayer, "Unknown!");
                }
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

static void BleApp_PrintMeasurementResults(deviceId_t deviceId, localizationAlgoResult_t *pResult)
{
    uint16_t qInt = 0U;
    uint16_t qFrac = 0U;

    if (mProcedureCount == mRangeSettings[deviceId].maxNumProcedures)
    {
#if defined(gAppUseRADEAlgorithm_d) && (gAppUseRADEAlgorithm_d == 1)
        if ((pResult->algorithm & eMciqAlgoEmbedRADE) != 0U)
        {
            ChipLogProgress(DeviceLayer, "Peer: %d", (uint8_t)deviceId);

            if (pResult->radeError != 0U)
            {
                ChipLogProgress(DeviceLayer, "RADE Error: %d", pResult->radeError);
            }
            else if (pResult->resultRADE.dqiIntegerPart == 0U)
            {
                ChipLogProgress(DeviceLayer, "Low Quality data for RADE! Quality indicator is 0!");
            }
            else if (pResult->resultRADE.distanceIntegerPart > gMaxDistanceMeters_c)
            {
                ChipLogProgress(DeviceLayer, "Outlier RADE distance result - discarded. Check gMaxDistanceMeters_c value.");
            }
            else
            {
                ChipLogProgress(DeviceLayer, "Distance (RADE): %d.%d m", pResult->resultRADE.distanceIntegerPart,
                                pResult->resultRADE.distanceDecimalPart);

                ChipLogProgress(DeviceLayer, "Quality: %d.%d", pResult->resultRADE.dqiIntegerPart, pResult->resultRADE.dqiDecimalPart);
            }
        }
#endif /* gAppUseRADEAlgorithm_d */

#if defined(gAppUseCDEAlgorithm_d) && (gAppUseCDEAlgorithm_d == 1)
        if ((pResult->algorithm & eMciqAlgoEmbedCDE) != 0U)
        {
            ChipLogProgress(DeviceLayer, "Peer: %d", (uint8_t)deviceId);

            ChipLogProgress(DeviceLayer, "Distance (CDE): %d.%d m", pResult->resultCDE.distanceIntegerPart,
                                pResult->resultCDE.distanceDecimalPart);

            ChipLogProgress(DeviceLayer, "Quality: %d.%d", pResult->resultCDE.dqiIntegerPart, pResult->resultCDE.dqiDecimalPart);
        }
#endif /* gAppUseCDEAlgorithm_d */

#if defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1)
        ChipLogProgress(DeviceLayer, "Time information:");

        if (pResult->csConfigDuration != 0)
        {
            ChipLogProgress(DeviceLayer, "CS Config: %d ms", (int) pResult->csConfigDuration/1000);
        }
        if (pResult->csProcedureDuration != 0)
        {
            ChipLogProgress(DeviceLayer, "CS Procedure: %d ms", (int) pResult->csProcedureDuration/1000);
        }
        if (pResult->rasTransferDuration != 0)
        {
            ChipLogProgress(DeviceLayer, "RAS transfer: %d ms", (int) pResult->rasTransferDuration/1000);
        }
        if (pResult->algoDuration != 0)
        {
            ChipLogProgress(DeviceLayer, "Localization algorithm: %d ms", (int) pResult->algoDuration/1000);
        }
#endif /* defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1) */

        /* Print RTT information */
        ChipLogProgress(DeviceLayer, "Peer: %d", (uint8_t)deviceId);

        if (pResult->rttResult.dm_ad <= 0)
        {
            pResult->rttResult.dm_ad = -pResult->rttResult.dm_ad;
        }

        /* Round to nearest integer */
        pResult->rttResult.dm_ad += 3277;
        qInt = (uint16_t)((uint32_t)pResult->rttResult.dm_ad >> 16U);
        qFrac = (uint16_t)((((uint32_t)pResult->rttResult.dm_ad & 0x0000FFFFU)*10U) >> 16U);

        if (pResult->rttResult.dm_ad <= 0)
        {
            ChipLogProgress(DeviceLayer, "RTT Distance: -%d.%d m", qInt, qFrac);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "RTT Distance: %d.%d m", qInt, qFrac);
        }
        
        ChipLogProgress(DeviceLayer, "Success Rate: %d", pResult->rttResult.dm_sr);
    }
}