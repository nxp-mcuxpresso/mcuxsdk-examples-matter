# Matter NXP Lighting Example Application

- [Matter NXP Lighting Example Application](#matter-nxp-lighting-example-application)
  - [Overview](#overview)
  - [Supported Platforms](#supported-platforms)
  - [Environment Setup, Building, and Testing](#environment-setup-building-and-testing)
  - [Data Model](#data-model)
  - [Supported Configurations](#supported-configurations)

<a name="overview"></a>

## Overview

This reference application implements an On/Off Light device type. It uses board
buttons or `matter-cli` for user input and LEDs for state feedback. You can use
this example as a reference for creating your own application.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype application that demonstrates device commissioning and
different cluster control.

The light bulb device communicates with clients over a low-power, 802.15.4 Thread network.
It can be commissioned into an existing Matter network using a controller such
as `chip-tool`.

This example implements a `User-Intent Commissioning Flow`, meaning that the user
is required to press a button on the device in order to get it ready for commissioning. The
initial commissioning is usually performed using the `ble-thread` pairing method.

The Thread network dataset will be transferred on the device using a secure
session over Bluetooth LE. In order to start the commissioning process, the user
must enable BLE advertising on the device manually. To pair successfully, the
commissioner must know the commissioning information corresponding to the
device: setup passcode and/or discriminator. This data is usually encoded within a
QR code or printed to the device's UART console.

<a name="supported-platforms"></a>

## Supported Platforms

The On/Off Light example is supported on the following platforms:

| NXP platform        | Dedicated readme                                                             |
| ------------------- | ---------------------------------------------------------------------------- |
| MCXW71              | [NXP MCXW71 Guide](../../../middleware/matter/docs/platforms/nxp/nxp_mcxw71_guide.md)          |
| MCXW72              | [NXP MCXW72 Guide](../../../middleware/matter/docs/platforms/nxp/nxp_mcxw72_guide.md)          |

For details on platform-specific requirements and configurations, please refer
to the respective platform's readme.

## Environment Setup, Building, and Testing

All the information required to set up the environment, build the application,
and test, are available in the [Matter Documentation for NXP MCU platforms](https://docs.mcuxpresso.nxp.com/matter/latest/html/index.html)

## Data Model

The application uses an NXP specific data model file:

| Path                      | Description                             |
| ------------------------- | --------------------------------------- |
| `zap/lighting-on-off.zap` | Data model for On/Off Light device type |

The data model can be changed by simply replacing the gn `deps` statement
corresponding to data model target.

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | Default | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_ftd` | Thread Full Thread Device (FTD) configuration | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_ftd.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_ftd_factory_data` | Thread FTD configuration with factory data provisioning | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_ftd_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_ftd_ota` | Thread FTD configuration with OTA update capability | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_ftd_ota.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_ftd_ota_factory_data` | Thread FTD configuration with OTA updates and factory data | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_ftd_ota_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_ftd_se05x_default_crypto_no_cli` | Thread Full Thread Device (FTD) configuration and SE05X default crypto CLI disabled to ease MATTER logging view on main UART console | frdmmcxw72@cm33_core0 | `prj_thread_ftd.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_CHIP_SE05X=y`<br>`-DCONFIG_CHIP_LIB_SHELL=n` |
| `thread_ftd_ota_se05x_default_crypto_no_cli` | Thread FTD configuration with OTA updates and SE05X default crypto CLI disabled to ease MATTER logging view on main UART console | frdmmcxw72@cm33_core0 | `prj_thread_ftd_ota.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_CHIP_SE05X=y`<br>`-DCONFIG_CHIP_LIB_SHELL=n` |
| `thread_ftd_se05x_default_crypto_no_cli_factory_data` | Thread Full Thread Device (FTD) configuration and SE05X default crypto, DA key, CLI disabled to ease MATTER logging view on main UART console | frdmmcxw72@cm33_core0 | `prj_thread_ftd_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_CHIP_SE05X=y`<br>`-DCONFIG_CHIP_LIB_SHELL=n` |
| `thread_ftd_ota_se05x_default_crypto_no_cli_factory_data` | Thread Full Thread Device (FTD) configuration with OTA updates and SE05X default crypto, DA key, CLI disabled to ease MATTER logging view on main UART console | frdmmcxw72@cm33_core0 | `prj_thread_ftd_ota_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_CHIP_SE05X=y`<br>`-DCONFIG_CHIP_LIB_SHELL=n` |
| `thread_ftd_se05x_default_crypto_with_cli_gpio` | Thread Full Thread Device (FTD) configuration and SE05X default crypto CLI enabled with GPIO control for SE05X power on/off | frdmmcxw72@cm33_core0 | `prj_thread_ftd.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_CHIP_SE05X=y`<br>`-DCONFIG_CHIP_LIB_SHELL=y`<br>`-DCONFIG_SE05X_HOST_GPIO=y` |
