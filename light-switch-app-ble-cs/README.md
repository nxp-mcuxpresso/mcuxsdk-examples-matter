# Matter NXP Light Switch Bluetooth Channel Sounding Application

## Overview

The NXP Light Switch BLE CS example is based on
[Matter](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype of a light switch device that also has BLE CS functionality
which alows it to know the distance to another BLE CS device.

To build the NXP Light Switch BLE CS example follow the steps:
- Setup the environment
- Download and patch the bluetooth cs restricted access sdk zip
- Build the MCXW72 NXP Light Switch BLE CS example
- Build the loc_user device BLE CS example
- Matter BLE CS example

## Environment Setup

All the information required to set up the environment, build the application,
and test it can be found in the common readme for NXP platforms :

-   NXP FreeRTOS Platforms : Refer to the
    [Matter NXP Examples Guide for FreeRTOS platforms](../../../middleware/matter/docs/platforms/nxp/nxp_examples_freertos_platforms.md)


## Download and patch BLE CS restricted access SDK zip

- Go to [MCUXpresso SDK Builder](https://kex-stage.nxp.com/).
- Click `Select Development Board`, type `MCX-W72-EVK` in the input bar and select the board
- The selection will appear in the right, select the 25.09.00 SDK version with Tag v25.09.00_RC2 and click `BUILD SDK`. [Select board](./main/images/board-select.png)
- In the new page select `MCUXpresso IDE` in the `Toolchain/IDE` tab, desired `Host OS` and select all SDK packages, then click `BUILD SDK`.
- Download the zip once ready. [Build SDK](./main/images/sdk-builder.png)
- Unzip and open file `middleware/wireless/bluetooth/application/common/lcl/app_localization_algo.c`. Comment lines 41-51, 68-81, 86-100.
- Copy the `bluetooth` folder from the SDK ZIP into the sdk-next path `mcuxsdk/middleware/wireless/bluetooth_cs`.

## Build the MCXW72 NXP Light Switch BLE CS example

- Activate the matter environment
- The example is located into `examples/light-switch-app-ble-cs`
- Use `west list` to see the building variants. `mcxw72evk` or `frdmmcxw72` are the available boards.
- Build the app: eg `west build -d build_matter -b mcxw72evk examples/matter_examples/light-switch-app-ble-cs/mcux -DCONF_FILE=<absolute_path_to>/prj_thread_ftd.conf -DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n -Dcore_id=cm33_core0 -DCONFIG_CHIP_LIB_SHELL=n`.
- `-DCONFIG_CHIP_LIB_SHELL=n` is added to redirect the matter logs to the usb serial, this is easier for distance measurement visualisation. If this config is removed, matter cli is available on the usb serial and a USB-UART bridge is needed to see the logs, see `Additional UART interface` from [README.md](../../../middleware/matter/examples/lighting-app/nxp/mcxw72/README.md)
- The application binary will be located into `./build_matter/app.bin`.
- Use the NBU binary from the SDK zip or sdk-next: `middleware/wireless/ieee-802.15.4/bin/mcxw72/mcxw72_nbu_ble_full_15_4_dyn.bin`
- Use the instructions from [README.md](../../../middleware/matter/examples/lighting-app/nxp/mcxw72/README.md) for flashing on the MCXW72 board.

## Build the loc_user device BLE CS example

- Download [MCUXpresso-IDE](https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE).
- Open MCUXpresso-IDE, close Welcome tab.
- Drag and drop the SDK zip intro the `Installed SDKs` window. [Install SDK](./main/images/install-sdk.png)
- In `Project Explorer` on the right select `Import SDK example(s)...`, click on mcxw72evk board, click Next.
- Select loc_user_device_bm project in the SDK Import Wizard window and click Finish. [Import project](./main/images/select-project.png)
- In the Project Explorer go to `source/app_preinclude.h` and set `gAppUsePrivacy_d` to 0 and save. [Disable privacy](./main/images/disable-privacy.png)
- In the Project Explorer right-click on project and select `Build Project`. [Build project](./main/images/build-project.png)
- After the project is built, go into Debug, right click on `mcxw72evk_loc_user_device_bm.axf`, go to Binary Utilities > Create binary to convert built file to binary format. [Convert to binary](./main/images/create-binary.png)
- Use this application binary: `Debug/mcxw72evk_loc_user_device_bm.bin`.
- Use the NBU binary from the SDK zip or sdk-next: `middleware/wireless/ble_controller/bin/mcxw72_nbu_ble_all_hosted.bin`.
- Use the instructions from [README.md](../../../middleware/matter/examples/lighting-app/nxp/mcxw72/README.md) for flashing on the MCXW72 board.

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
