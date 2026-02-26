# CHIP NXP EVSE Application

## Overview

The NXP EVSE example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype of an EVSE device which demonstrates device
commissioning and different cluster control.

### Supported platforms

The EVSE example is supported on the following platforms :

| NXP platform        | Dedicated readme                                                             |
| ------------------- | ---------------------------------------------------------------------------- |
| RW61x (FreeRTOS OS) | [NXP RW61x (FreeRTOS) Guide](../../../middleware/matter/docs/platforms/nxp/nxp_rw61x_guide.md) |

For details on platform-specific requirements and configurations, please refer
to the respective platform's readme.

## Environment Setup, Building, and Testing

All the information required to set up the environment, build the application,
and test, are available in the [Matter Documentation for NXP MCU platforms](https://docs.mcuxpresso.nxp.com/matter/latest/html/index.html)

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | frdmrw612, rdrw612bga | Default | None |
| `wifi_factory_data` | Wi-Fi configuration with factory data provisioning | frdmrw612, rdrw612bga | `prj_wifi_fdata.conf` | None |
| `wifi_ota` | Wi-Fi configuration with OTA update capability | frdmrw612, rdrw612bga | `prj_wifi_ota.conf` | None |
| `wifi_ota_factory_data` | Wi-Fi configuration with OTA updates and factory data | frdmrw612, rdrw612bga | `prj_wifi_ota_fdata.conf` | None |
