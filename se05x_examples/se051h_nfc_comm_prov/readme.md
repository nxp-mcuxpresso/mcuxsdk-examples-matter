# NFC Commissioning Provision example

The example can be used to provision the credentials required for NFC / unpowered commissioning of matter devices.
The example is be used with SE051H samples updated with NFC commissioning applet.

-   The following credentials are updated using this example :

    | Content                                   | Key id         | Type               |
    | ----------------------------------        | ----------     | ------------------ |
    | PBKDF Parameters                          | 0x7FFF3002     | Binary File        |
    | Device attestation certificate            | 0x7FFF3003     | Binary file        |
    | Product attestation Authority certificate | 0x7FFF3004     | NIST-256 Key Pair  |
    | Device attestation key pair               | 0x7FFF3007     | Binary file        |
    | Device attestation TBS                    | 0x7FFF3005     | Binary file        |
    | Select response                           | 0x7FFF3001     | Binary file        |
    | Node Operational key pair                 | 0x7FFF3101     | NIST-256 Key Pair  |
    | Node Operational certificate              | 0x7FFF3201     | Binary file        |
    | Root certificate                          | 0x7FFF3301     | Binary file        |
    | Wi-Fi Credentials                         | 0x7FFF3401     | Binary file        |
    | Access Control List                       | 0x7FFF3501     | Binary file        |
    | Identity Protection Epoch Key             | 0x7FFF3601     | Binary file        |
    |                                           |                |                    |
    | Basic Information Cluster                 | 0x7FFE0028     | Binary file        |
    | General commissioning Cluster             | 0x7FFE0030     | Binary file        |
    | Operational Credential Cluster            | 0x7FFE003E     | Binary file        |
    | Access Control Cluster                    | 0x7FFE001F     | Binary file        |
    | Network Commissioning Cluster             | 0x7FFE0031     | Binary file        |
    |                                           |                |                    |
    | Vendor reserved binary file               | 0x7FFF3006     | Binary file        |
    | Descriptive cluster (End point 0)         | 0x7FFE001D     | Binary file        |
    | Descriptive cluster (End point 1)         | 0x00017FFE001D | Binary file        |


NOTE: The example can also be used to provision the QR code into T4T applet.

# Building

The example can be built with

## Matter (for RW612) CMake Build system as

```
user@ubuntu:~/sdk-next/mcuxsdk$ west build -d <out_dir> -b frdmrw612 examples/matter_examples/se05x_examples/se051h_nfc_comm_prov/mcux
```

## Matter (for FRDMW72) CMake Build system as

```
user@ubuntu:~/sdk-next/mcuxsdk$ west build -d <out_dir> -b frdmmcxw72 examples/matter_examples/se05x_examples/se051h_nfc_comm_prov/mcux -Dcore_id=cm33_core0 -DCONFIG_CHIP_SE05X=y
```

## Matter (for RT1060) CMake Build system as

```
user@ubuntu:~/sdk-next/mcuxsdk$ west build -d <out_dir> -b evkcmimxrt1060 examples/matter_examples/se05x_examples/se051h_nfc_comm_prov/mcux
```

> [!IMPORTANT]
> Adapt the above commands to the i.MX, RW612 and RT1060 build commands accordingly.

# Usage

When using on supported MCUs, you can configure the provisioning options using Kconfig build options.

## Configuration Options

The following Kconfig options are available:

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `-DCONFIG_SE05X_DO_RESET` | bool | n | Delete all provisioned data (QR code is not deleted) |
| `-DCONFIG_SE05X_DO_EC_KEY_PROVISION` | bool | n | Provision EC key for applet session |
| `-DCONFIG_SE05X_DO_USER_ID_PROVISION` | bool | n | Provision User ID for applet session |
| `-DCONFIG_SE05X_DO_AES_KEY_PROVISION` | bool | n | Provision AES key for applet session |
| `-DCONFIG_SE05X_ONLY_T4T_PROVISION` | bool | n | Provision only QR code in T4T applet |
| `-DCONFIG_SE05X_TP_SPAKE_PASSCODE_SET_NO` | int | 1 | Trust Provisioned pass-code set number (1-3) |
| `-DCONFIG_SE05X_TP_SPAKE_ITER_TO_BE_USED` | int | 1000 | Trust Provisioned iteration count (1000, 5000, 10000, 50000, 100000) |
| `-DCONFIG_SE05X_DEVICE_NETWORK_TYPE_WIFI` | bool | y | Enable WiFi network interface for NFC commissioning |
| `-DCONFIG_SE05X_DEVICE_NETWORK_TYPE_THREAD` | bool | n | Enable Thread network interface for NFC commissioning |
| `-DCONFIG_SE05X_DEVICE_NETWORK_TYPE_ETHERNET` | bool | n | Enable Ethernet network interface for NFC commissioning |
| `-DCONFIG_SE05X_PROVISION_WITH_POLICY` | bool | n | Enable provisioning with policy |


The example has the default DAC keys and certificates from MATTER SDK. To use custom DAC keys and certificates, modify the header file contents (file - mcuxsdk/middleware/matter/third_party/simw-top-mini/repo/demos/se051h_nfc_comm_prov/common/se051h_nfc_comm_prov.h),

```c
#define DAC_CERTIFICATE                                                        \
  0x31, 0x00, 0xED, 0x01, 0x30, 0x82, 0x01, 0xE9, 0x30, 0x82, 0x01, 0x8E,      \
...
...
```

```c
#define DA_KEY_PAIR_DATA                                                       \
  0x30, 0x77, 0x02, 0x01, 0x01, 0x04, 0x20, 0xCC, 0xCF, 0x9D, 0xC7, 0x05,      \
      0x0E, 0xF5, 0xD9, 0x0B, 0xE4, 0x57, 0x07,
...
...
```

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | None |
| `scp03` | NFC commissioning provisioning example with platform scp03 authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03=y` |
| `aes_key` | NFC commissioning provisioning example with aes Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_AES_KEY=y` |
| `reset_with_aes_key_session` | Delete all provisioned data (QR code is not deleted) with aes Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_AES_KEY=y`<br>`-DCONFIG_SE05X_DO_RESET=y` |
| `reset_plain` | Delete all provisioned data (QR code is not deleted) | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_DO_RESET=y` |
| `plain_aes_key_provision` | Provision AES key in to SE05x with plain session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_DO_AES_KEY_PROVISION=y` |
| `network_type_thread` | Enable thread network interface for NFC commissioning | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_DEVICE_NETWORK_TYPE_THREAD=y` |
