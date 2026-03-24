# Example To Get Pass-code of SE051H secure element

SE051H samples has a trust provisioned binary file (0x7FFF2000) containing the pass-code and salt for NFC commissioning.
The example can be used to read the pass-code value from this binary file.

# Building

The example can be built with

## Matter (for RW612) CMake Build system as

```
user@ubuntu:~/sdk-next/mcuxsdk$ west build -d <out_dir> -b frdmrw612 examples/matter_examples/se05x_examples/se05x_get_passcode/mcux
```

## Matter (for FRDMW72) CMake Build system as

```
user@ubuntu:~/sdk-next/mcuxsdk$ west build -d <out_dir> -b frdmmcxw72 examples/matter_examples/se05x_examples/se05x_get_passcode/mcux -Dcore_id=cm33_core0 -DCONFIG_CHIP_SE05X=y
```

## Matter (for RT1060) CMake Build system as

```
user@ubuntu:~/sdk-next/mcuxsdk$ west build -d <out_dir> -b evkcmimxrt1060 examples/matter_examples/se05x_examples/se05x_get_passcode/mcux
```

# Usage

When using on supported MCUs, modify the main file defines (examples - sdk-next/mcuxsdk/middleware/matter/third_party/simw-top-mini/repo/demos/se05x_get_passcode/mcu/main.cpp)

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | None |
| `scp03` | Get passcode example with platform scp03 authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03=y` |
| `user_id` | Get passcode example with user Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_USER_ID=y` |
| `aes_key` | Get passcode example with aes Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_AES_KEY=y` |
| `ec_key` | Get passcode example with ec Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_EC_KEY=y` |
| `scp03_user_id` | Get passcode example with SCP03 + user Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03_USER_ID=y` |
| `scp03_aes_key` | Get passcode example with SCP03 + aes key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03_AES_KEY=y` |
| `scp03_ec_key` | Get passcode example with SCP03 + ec key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03_EC_KEY=y` |
