# Matter NXP Light Switch Bluetooth Channel Sounding Application

## Overview

The NXP Light Switch BLE CS example is based on
[Matter](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype of a light switch device that also has BLE CS functionality
which alows it to know the distance to another BLE CS device.

To build the NXP Light Switch BLE CS example follow the steps:
- Setup the environment
- Build the MCXW72 NXP Light Switch BLE CS example
- Build the loc_user_device BLE CS example
- Matter BLE CS example

## Environment Setup

All the information required to set up the environment, build the application,
and test, are available in the [Matter Documentation for NXP MCU platforms](https://docs.mcuxpresso.nxp.com/matter/latest/html/index.html)

## Build the MCXW72 NXP Light Switch BLE CS example

- Activate the matter environment
- The example is located into `examples/matter_examples/light-switch-app-ble-cs`
- Use `west list` to see the building variants. `mcxw72evk` or `frdmmcxw72` are the available boards.
- Build the app. E.g. `west build -d build_matter -b frdmmcxw72 examples/matter_examples/light-switch-app-ble-cs/mcux -DCONF_FILE_NAME=prj_thread_ftd.conf -DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n -Dcore_id=cm33_core0 -DCONFIG_CHIP_LIB_SHELL=n`.
- `-DCONFIG_CHIP_LIB_SHELL=n` is added to redirect the matter logs to the usb serial, this is easier for distance measurement visualisation. If this config is removed, matter cli is available on the usb serial and a USB-UART bridge is needed to see the logs, see `UART Ports` from [README.md](../../../middleware/matter/docs/platforms/nxp/nxp_mcxw72_guide.md)
- The application binary will be located into `./build_matter/app.bin`.
- Use the NBU binary from the sdk: `middleware/wireless/ieee-802.15.4/bin/mcxw72/mcxw72_nbu_ble_full_15_4_dyn.bin`
- Use the instructions from [README.md](../../../middleware/matter/docs/platforms/nxp/nxp_mcxw72_guide.md) for flashing on the MCXW72 board.

## Build the loc_user_device BLE CS example

- Open and edit the appropriate app_preinclude.h file for the loc_user_device_bm example (`middleware/wireless/bluetooth/boards/mcxw72evk/bluetooth/loc_user_device/app_preinclude.h`)
- Set `gAppUsePrivacy_d` to `0`
- Set `gAppIsPeripheral_d` to `0U`
- Set `gCsDefaultRole_c` to `gCsRoleInitiator_c`
- The example is located into `examples/wireless_examples/bluetooth/loc_user_device/bm`
- Use `west list` to see the building variants. `mcxw72evk` or `frdmmcxw72` are the available boards.
- Build the app. E.g. `west build -d loc_user_device -b mcxw72evk examples/wireless_examples/bluetooth/loc_user_device/bm -Dcore_id=cm33_core0`.
- The application binary will be located into `./loc_user_device/loc_user_device_bm_cm33_core0.elf`.
- Use the NBU binary from the sdk: `middleware/wireless/ble_controller/bin/mcxw72_nbu_ble_hosted.bin`.
- Use the instructions from [README.md](../../../middleware/matter/docs/platforms/nxp/nxp_mcxw72_guide.md) for flashing on the MCXW72 board.

## Matter BLE CS example

- Connect the MCXW72 light-switch-app-ble-cs device and the MCXW72 loc_user_device_bm and open serial terminals for both. If the matter cli is used, the matter logs will be available on the second serial, see [Build section](#build-the-loc_user-device-ble-cs-example).
- Commission the MCXW72 light-switch-app-ble-cs device into the Matter fabric (press SW2 or type `mattercommissioning on` on the serial terminal if matter cli is used).
- Start BLE advertising again on MCXW72 light-switch-app-ble-cs (press SW2 or type `startblecsadv` on the serial terminal if matter cli is used).
- On MCXW72 loc_user_device_bm console type `factoryreset` (optional after first connection), `sb`.
- The distance measurements will appear on the MCXW72 light-switch-app-ble-cs logs.
- On MCXW72 loc_user_device_bm console type `tdm 0` to trigger further distance measurements.
- [Example log](./main/images/distance-measurements.png)

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | Default | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_ftd` | Thread Full Thread Device (FTD) configuration with BLE CS functionality | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_ftd.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_ftd_factory_data` | Thread Full Thread Device (FTD) configuration with BLE CS functionality and factory data provisioning | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_ftd_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_ftd_ota` | Thread Full Thread Device (FTD) configuration with BLE CS functionality and OTA updates | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_ftd_ota.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
| `thread_ftd_ota_factory_data` | Thread Full Thread Device (FTD) configuration with BLE CS functionality, OTA updates and factory data provisioning | frdmmcxw72@cm33_core0, mcxw72evk@cm33_core0 | `prj_thread_ftd_ota_fdata.conf` | `-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` |
