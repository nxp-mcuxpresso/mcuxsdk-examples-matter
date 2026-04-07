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

When using on supported MCUs, you can configure the passcode set number using Kconfig build options:
`TP_SPAKE_PASSCODE_SET_NO` (Possible values = 1,2,3).

Pass the configuration during build:

```
# For FRDMW72
west build -d <out_dir> -b frdmmcxw72 examples/matter_examples/se05x_examples/se05x_get_passcode/mcux -Dcore_id=cm33_core0 -DCONFIG_SE05X_TP_SPAKE_PASSCODE_SET_NO=1
```
Note: If no pass-code set number is passed, default pass-code set number is 1.

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | None |
| `scp03` | Get passcode example with platform scp03 authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03=y` |
| `aes_key` | Get passcode example with aes Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_AES_KEY=y` |
