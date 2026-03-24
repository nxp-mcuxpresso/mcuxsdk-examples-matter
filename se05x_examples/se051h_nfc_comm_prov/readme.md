# NFC Commissioning Provision example

The example can be used to provision the credentials required for NFC / unpowered commissioning of matter devices.
The example is be used with SE051H samples updated with NFC commissioning applet.

-   The following credentials are updated using this example :

    | Content                                   | Key id     | Type               |
    | ----------------------------------        | ---------- | ------------------ |
    | PBKDF Parameters                          | 0x7FFF3002 | Binary File        |
    | Device attestation certificate            | 0x7FFF3003 | Binary file        |
    | Product attestation Authority certificate | 0x7FFF3004 | NIST-256 Key Pair  |
    | Device attestation key pair               | 0x7FFF3007 | Binary file        |
    | Device attestation TBS                    | 0x7FFF3005 | Binary file        |
    | Select response                           | 0x7FFF3001 | Binary file        |
    | Node Operational key pair                 | 0x7FFF3101 | NIST-256 Key Pair  |
    | Node Operational certificate              | 0x7FFF3201 | Binary file        |
    | Root certificate                          | 0x7FFF3301 | Binary file        |
    | Wi-Fi Credentials                         | 0x7FFF3401 | Binary file        |
    | Access Control List                       | 0x7FFF3501 | Binary file        |
    | Identity Protection Epoch Key             | 0x7FFF3601 | Binary file        |
    |                                           |            |                    |
    | Basic Information Cluster                 | 0x7FFE0028 | Binary file        |
    | General commissioning Cluster             | 0x7FFE0030 | Binary file        |
    | Operational Credential Cluster            | 0x7FFE003E | Binary file        |
    | Access Control Cluster                    | 0x7FFE001F | Binary file        |
    | Network Commissioning CLuster             | 0x7FFE0031 | Binary file        |


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

When using on supported MCUs, modify the main file defines (examples - sdk-next/mcuxsdk/middleware/matter/third_party/simw-top-mini/repo/demos/se051h_nfc_comm_prov/mcu/main.cpp)

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | None |
| `scp03` | NFC commissioning provisioning example with platform scp03 authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03=y` |
| `user_id` | NFC commissioning provisioning example with user Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_USER_ID=y` |
| `aes_key` | NFC commissioning provisioning example with aes Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_AES_KEY=y` |
| `ec_key` | NFC commissioning provisioning example with ec Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_EC_KEY=y` |
| `scp03_user_id` | NFC commissioning provisioning example with SCP03 + user Key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03_USER_ID=y` |
| `scp03_aes_key` | NFC commissioning provisioning example with SCP03 + aes key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03_AES_KEY=y` |
| `scp03_ec_key` | NFC commissioning provisioning example with SCP03 + ec key authenticated session | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03_EC_KEY=y` |
