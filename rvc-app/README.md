# CHIP NXP Robotic Vacuum Cleaner Application

## Overview

The NXP Robotic Vacuum Cleaner example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype of a RVC device that demonstrates device commissioning and
different cluster control.

### Supported platforms

The Robotic Vacuum Cleaner example is supported on the following platforms :

| NXP platform        | Dedicated readme                                                             |
| ------------------- | ---------------------------------------------------------------------------- |
| RW61x (FreeRTOS OS) | [NXP RW61x (FreeRTOS) Guide](../../../middleware/matter/docs/platforms/nxp/nxp_rw61x_guide.md) |
| RT1170              | [NXP RT1170 Guide](../../../middleware/matter/docs/platforms/nxp/nxp_rt1170_guide.md)          |
| RT1060              | [NXP RT1060 Guide](../../../middleware/matter/docs/platforms/nxp/nxp_rt1060_guide.md)          |

For details on platform-specific requirements and configurations, please refer
to the respective platform's readme.

## Environment Setup, Building, and Testing

All the information required to set up the environment, build the application,
and test it can be found in the common readme for NXP platforms :

-   NXP FreeRTOS Platforms : Refer to the
    [CHIP NXP Examples Guide for FreeRTOS platforms](../../../docs/platforms/nxp/nxp_examples_freertos_platforms.md)

> Note : Matter-over-WiFi + Thread Border Router configuration is not supported
> in this application.

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | evkbmimxrt1060, evkbmimxrt1170@cm7, evkcmimxrt1060, frdmrw612, rdrw612bga | Default | None |
| `wifi_iw610` | Wi-Fi configuration with IW610 transceiver | evkcmimxrt1060 | `prj_wifi.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y`<br>`-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ll_m2=y` |
| `wifi_iw416` | Wi-Fi configuration with IW416 transceiver | evkbmimxrt1060 | `prj_wifi.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW416=y` |
| `wifi_factory_data` | Wi-Fi configuration with factory data provisioning | frdmrw612, rdrw612bga | `prj_wifi_fdata.conf` | None |
| `wifi_iw612_fdata` | Wi-Fi configuration with IW612 transceiver and factory data | evkcmimxrt1060, evkbmimxrt1170@cm7 | `prj_wifi_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y` |
| `wifi_iw610_fdata` | Wi-Fi configuration with IW610 transceiver and factory data | evkcmimxrt1060 | `prj_wifi_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y`<br>`-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ll_m2=y` |
| `wifi_iw416_fdata` | Wi-Fi configuration with IW416 transceiver and factory data | evkbmimxrt1060 | `prj_wifi_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW416=y` |
| `wifi_ota` | Wi-Fi configuration with OTA update capability | frdmrw612, rdrw612bga | `prj_wifi_ota.conf` | None |
| `wifi_iw612_ota` | Wi-Fi configuration with IW612 transceiver and OTA updates | evkcmimxrt1060, evkbmimxrt1170@cm7 | `prj_wifi_ota.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y` |
| `wifi_iw610_ota` | Wi-Fi configuration with IW610 transceiver and OTA updates | evkcmimxrt1060 | `prj_wifi_ota.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y`<br>`-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ll_m2=y` |
| `wifi_ota_factory_data` | Wi-Fi configuration with OTA updates and factory data | frdmrw612, rdrw612bga | `prj_wifi_ota_fdata.conf` | None |
| `wifi_iw612_ota_factory_data` | Wi-Fi configuration with IW612 transceiver, OTA updates, and factory data | evkcmimxrt1060, evkbmimxrt1170@cm7 | `prj_wifi_ota_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y` |
| `wifi_iw610_ota_factory_data` | Wi-Fi configuration with IW610 transceiver, OTA updates, and factory data | evkcmimxrt1060 | `prj_wifi_ota_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y`<br>`-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ll_m2=y` |
| `thread_ftd` | Thread Full Thread Device (FTD) configuration | frdmrw612, rdrw612bga | `prj_thread_ftd.conf` | None |
| `thread_ftd_iw612` | Thread FTD configuration with IW612 transceiver | evkcmimxrt1060, evkbmimxrt1170@cm7 | `prj_thread_ftd.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y` |
| `thread_ftd_iw610` | Thread FTD configuration with IW610 transceiver | evkcmimxrt1060 | `prj_thread_ftd.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y`<br>`-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ll_m2=y` |
| `thread_ftd_k32w0` | Thread FTD configuration with K32W061 transceiver | evkbmimxrt1060 | `prj_thread_ftd.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.K32W061_transceiver=y` |
| `thread_ftd_factory_data` | Thread FTD configuration with factory data | frdmrw612, rdrw612bga | `prj_thread_ftd_fdata.conf` | None |
| `thread_ftd_iw612_factory_data` | Thread FTD configuration with IW612 transceiver and factory data | evkcmimxrt1060, evkbmimxrt1170@cm7 | `prj_thread_ftd_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y` |
| `thread_ftd_iw610_factory_data` | Thread FTD configuration with IW610 transceiver and factory data | evkcmimxrt1060 | `prj_thread_ftd_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y`<br>`-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ll_m2=y` |
| `thread_ftd_k32w0_factory_data` | Thread FTD configuration with K32W061 transceiver and factory data | evkbmimxrt1060 | `prj_thread_ftd_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.K32W061_transceiver=y` |
| `thread_ftd_ota` | Thread FTD configuration with OTA updates | frdmrw612, rdrw612bga | `prj_thread_ftd_ota.conf` | None |
| `thread_ftd_iw612_ota` | Thread FTD configuration with IW612 transceiver and OTA updates | evkcmimxrt1060, evkbmimxrt1170@cm7 | `prj_thread_ftd_ota.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y` |
| `thread_ftd_iw610_ota` | Thread FTD configuration with IW610 transceiver and OTA updates | evkcmimxrt1060 | `prj_thread_ftd_ota.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y`<br>`-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ll_m2=y` |
| `thread_ftd_ota_factory_data` | Thread FTD configuration with OTA updates and factory data | frdmrw612, rdrw612bga | `prj_thread_ftd_ota_fdata.conf` | None |
| `thread_ftd_iw612_ota_factory_data` | Thread FTD configuration with IW612 transceiver, OTA updates, and factory data | evkcmimxrt1060, evkbmimxrt1170@cm7 | `prj_thread_ftd_ota_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y` |
| `thread_ftd_iw610_ota_factory_data` | Thread FTD configuration with IW610 transceiver, OTA updates, and factory data | evkcmimxrt1060 | `prj_thread_ftd_ota_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X=y`<br>`-DCONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ll_m2=y` |
