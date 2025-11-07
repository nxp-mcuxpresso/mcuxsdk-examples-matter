# CHIP NXP Light Switch Combo Application

## Overview

The NXP Light Switch Combo example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype of a light switch combo device that demonstrates device commissioning and
different cluster control.

### Supported platforms

The Light Switch Combo example is supported on the following platforms :

| NXP platform        | Dedicated readme                                                             |
| ------------------- | ---------------------------------------------------------------------------- |
| RW61x (FreeRTOS OS) | [NXP RW61x (FreeRTOS) Guide](../../../docs/platforms/nxp/nxp_rw61x_guide.md) |
| MCXW71              | [NXP MCXW71 Guide](../../../docs/platforms/nxp/nxp_mcxw71_guide.md)          |

For details on platform-specific requirements and configurations, please refer
to the respective platform's readme.

## Environment Setup, Building, and Testing

All the information required to set up the environment, build the application,
and test it can be found in the common readme for NXP platforms :

-   NXP FreeRTOS Platforms : Refer to the
    [Matter Documentation for NXP MCU platforms](https://docs.mcuxpresso.nxp.com/matter/latest/html/index.html)

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | frdmrw612, rdrw612bga | Default | None |
| `wifi_factory_data` | Wi-Fi configuration with factory data provisioning | frdmrw612, rdrw612bga | `prj_wifi_fdata.conf` | None |
| `wifi_ota` | Wi-Fi configuration with OTA update capability | frdmrw612, rdrw612bga | `prj_wifi_ota.conf` | None |
| `wifi_ota_factory_data` | Wi-Fi configuration with OTA updates and factory data | frdmrw612, rdrw612bga | `prj_wifi_ota_fdata.conf` | None |
| `thread_ftd` | Thread Full Thread Device (FTD) configuration | frdmrw612, rdrw612bga | `prj_thread_ftd.conf` | None |
| `thread_ftd_factory_data` | Thread FTD configuration with factory data | frdmrw612, rdrw612bga | `prj_thread_ftd_fdata.conf` | None |
| `thread_ftd_ota` | Thread FTD configuration with OTA updates | frdmrw612, rdrw612bga | `prj_thread_ftd_ota.conf` | None |
| `thread_ftd_ota_factory_data` | Thread FTD configuration with OTA updates and factory data | frdmrw612, rdrw612bga | `prj_thread_ftd_ota_fdata.conf` | None |
| `thread_ftd_wifi_border_router_ota` | Thread FTD configuration with Wi-Fi border router and OTA updates | frdmrw612, rdrw612bga | `prj_thread_ftd_wifi_br_ota.conf` | None |
| `thread_ftd_wifi_border_router_ota_factory_data` | Thread FTD configuration with Wi-Fi border router, OTA updates, and factory data | frdmrw612, rdrw612bga | `prj_thread_ftd_wifi_br_ota_fdata.conf` | None |
