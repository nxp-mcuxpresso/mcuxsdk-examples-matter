# Matter NXP Contact Sensor Example Application

- [Matter NXP Contact Sensor Example Application](#matter-nxp-contact-sensor-example-application)
  - [Overview](#overview)
  - [Supported Platforms](#supported-platforms)
  - [Environment Setup, Building, and Testing](#environment-setup-building-and-testing)
  - [Data Model](#data-model)
  - [Long Idle Time ICD Support](#long-idle-time-icd-support)
  - [Low Power](#low-power)
  - [Supported Configurations](#supported-configurations)

<a name="overview"></a>

## Overview

This reference application implements a Contact Sensor device type. It uses
board buttons or `matter-cli` for user input and LEDs for state feedback. You
can use this example as a reference for creating your own application.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype application that demonstrates device commissioning and
different cluster control.

This contact sensor communicates with clients over a low-power, 802.15.4 Thread network.
It can be commissioned into an existing Matter network using a controller, such
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

The Contact Sensor example is supported on the following platforms :

| NXP platform | Dedicated readme                                                                      |
| ------------ | ------------------------------------------------------------------------------------- |
| MCXW71       | [NXP MCXW71 Guide](../../../middleware/matter/docs/platforms/nxp/nxp_mcxw71_guide.md) |
| MCXW72       | [NXP MCXW72 Guide](../../../middleware/matter/docs/platforms/nxp/nxp_mcxw72_guide.md) |

For details on platform-specific requirements and configurations, please refer
to the respective platform's readme.

## Environment Setup, Building, and Testing

All the information required to set up the environment, build the application,
and test, are available in the [Matter Documentation for NXP MCU platforms](https://docs.mcuxpresso.nxp.com/matter/latest/html/index.html)

<a name="data-model"></a>

## Data Model

There are two available data models that can be used by the application:

| Path                             | Description                    |
| -------------------------------- | ------------------------------ |
| `zap-lit/contact-sensor-app.zap` | Data model for LIT ICD support |
| `zap-sit/contact-sensor-app.zap` | Data model for SIT ICD support |

The data model is selected automatically by the build system based on the user ICD flavor build option (SIT or LIT).

<a name="long-idle-time-icd-support"></a>

## Long Idle Time ICD Support

By default, the application is compiled as a SIT ICD (Short Idle Time
Intermittently Connected Device).

The following is a list of ICD configuration build options. These options can be provided on the
build command line to override default values. Please refer to your [platform guide](#supported-platforms)
for details on the build process.

| Build option                            | Default value | Description                                                                                                |
| --------------------------------------- | ------------- | ---------------------------------------------------------------------------------------------------------- |
| `CONFIG_CHIP_ICD_SLOW_POLL_INTERVAL`    | 2000 (ms)     | OT Idle Mode poll interval duration                                                                        |
| `CONFIG_CHIP_ICD_FAST_POLLING_INTERVAL` | 500 (ms)      | OT Active Mode poll interval duration                                                                      |
| `CONFIG_CHIP_ICD_IDLE_MODE_DURATION`    | 600 (s)       | Idle Mode Interval duration                                                                                |
| `CONFIG_CHIP_ICD_ACTIVE_MODE_DURATION`  | 10000 (ms)    | Active Mode Interval duration                                                                              |
| `CONFIG_CHIP_ICD_ACTIVE_MODE_THRESHOLD` | 5000 (ms)     | Active Mode Threshold duration                                                                             |
| `CONFIG_CHIP_ICD_CLIENTS_PER_FABRIC`    | 2             | Registration slots per fabric                                                                              |
| `CONFIG_CHIP_ICD_DSLS_SUPPORT`          | n             | Enable LIT ICD DSLS support                                                                                |
| `CONFIG_CHIP_PERSISTENT_SUBSCRIPTIONS`  | y             | Try once to re-establish subscriptions from the server side after reboot. May be disabled for LIT use case |

If LIT ICD support is required then `prj_thread_mtd_low_power_lit.conf` build configuration
file should be used when building the application. This will automatically configure the
above parameters to define a LIT device. You can still override these values on the build
command line in case you want to use different values from the defaults.

| Build option                            | LIT ICD default value |
| --------------------------------------- | --------------------- |
| `CONFIG_CHIP_ICD_SLOW_POLL_INTERVAL`    | 15000 (ms)            |
| `CONFIG_CHIP_ICD_FAST_POLLING_INTERVAL` | 500 (ms)              |
| `CONFIG_CHIP_ICD_IDLE_MODE_DURATION`    | 3600 (s)              |
| `CONFIG_CHIP_ICD_ACTIVE_MODE_DURATION`  | 15000 (ms)            |
| `CONFIG_CHIP_ICD_ACTIVE_MODE_THRESHOLD` | 30000 (ms)            |

<a name="low-power"></a>

## Low Power

This example also offers the possibility to run in low power mode. This means
that the device will use deep sleep/power down most of the time to aggresively lower the power
consumption.

Low power support is enabled at build time when using the build configuration files
appropriate for low-power app configurations. Please refer to the
[Supported Configurations](#supported-configurations)
for details on the available build configurations.

In order to maintain low power consumption, the UI LEDs are disabled and
device log and Matter CLI will not be available. For debugging purposes device log can
be forced using the `CONFIG_LOG=y` build option, but this might affect low power timings.
Also, please note that once the application is flashed onto the board the debugger
might disconnect when the board enters low power.

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | Default | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_mtd` | Thread Minimal Thread Device (MTD) configuration | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_mtd_battery` | Thread Minimal Thread Device (MTD) configuration with battery diagnostics | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_CHIP_APP_BATTERY_MANAGER=y`<br>`-DCONFIG_CHIP_APP_BATTERY_MANAGER_FREERTOS=y` |
| `thread_mtd_low_power` | Thread MTD configuration with low power optimization | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_low_power.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_mtd_low_power_lit` | Thread MTD and Matter ICD LIT configuration with low power optimization | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_low_power_lit.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_mtd_low_power_battery_lit` | Thread MTD and Matter ICD LIT configuration with low power optimization and battery diagnostics | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_low_power_lit.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_CHIP_APP_BATTERY_MANAGER=y`<br>`-DCONFIG_CHIP_APP_BATTERY_MANAGER_FREERTOS=y` |
| `thread_mtd_factory_data` | Thread MTD configuration with factory data provisioning | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_mtd_ota` | Thread MTD configuration with OTA update capability | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_ota.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_mtd_ota_low_power` | Thread MTD configuration with OTA updates and low power optimization | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_ota_low_power.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_mtd_ota_factory_data` | Thread MTD configuration with OTA updates and factory data | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_ota_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_mtd_ota_factory_data_low_power` | Thread MTD configuration with OTA updates, factory data, and low power optimization | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_ota_fdata_low_power.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_mtd_lit_lowpower_se05x_default_crypto` | Thread MTD LIT Configuration with SE05x default crypto | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_low_power_lit.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_SE05X_HOST_GPIO=y`<br>`-DCONFIG_CHIP_SE05X=y` |
| `thread_mtd_lit_lowpower_se05x_default_crypto_ota` | Thread MTD LIT Configuration with SE05x default crypto and OTA update | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_low_power_lit.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_CHIP_OTA_REQUESTOR=y`<br>`-DCONFIG_SE05X_HOST_GPIO=y`<br>`-DCONFIG_CHIP_SE05X=y` |
| `thread_mtd_lit_lowpower_se05x_default_crypto_ota_factory_data` | Thread MTD LIT Configuration with SE05x default crypto, OTA update and Factory Data | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_mtd_low_power_lit.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n`<br>`-DCONFIG_CHIP_OTA_REQUESTOR=y`<br>`-DCONFIG_CHIP_FACTORY_DATA=y`<br>`-DCONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED=y`<br>`-DCONFIG_CHIP_FACTORY_DATA_GENERATE_SPAKE2_VERIFIER=y`<br>`-DCONFIG_SE05X_HOST_GPIO=y`<br>`-DCONFIG_CHIP_SE05X=y` |
