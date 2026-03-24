# SE05x Device Attestation Provision Example

The example can be used to provision SE05x secure element with device attestation key pair and certificate.

-   The following credentials are updated using this example :

    | Content                                   | Key id     | Type               |
    | ----------------------------------        | ---------- | ------------------ |
    | Device attestation key pair               | 0x7FFF3007 | NIST-256 Key Pair  |
    | Device attestation certificate            | 0x7FFF3003 | Binary file        |

> [!IMPORTANT]
> The certificate is prefixed with additional 4 bytes of TLV value. The SE05x device attestation class will exclude these 4 bytes on reading.

# Building

The example can be built with

## Matter (for RW612) CMake Build system as

```
user@ubuntu:~/sdk-next/mcuxsdk$ west build -d <out_dir> -b frdmrw612 examples/matter_examples/se05x_examples/se05x_dev_attest_key_prov/mcux
```

## Matter (for FRDMW72) CMake Build system as

```
user@ubuntu:~/sdk-next/mcuxsdk$ west build -d <out_dir> -b frdmmcxw72 examples/matter_examples/se05x_examples/se05x_dev_attest_key_prov/mcux/ -Dcore_id=cm33_core0 -DCONFIG_CHIP_SE05X=y
```

## Matter (for RT1060) CMake Build system as

```
user@ubuntu:~/sdk-next/mcuxsdk$ west build -d <out_dir> -b evkcmimxrt1060 examples/matter_examples/se05x_examples/se05x_dev_attest_key_prov/mcux/
```

## Supported Configurations

| Variant | Description | Supported Boards | Project Configuration File | Extra Args |
|---------|-------------|------------------|----------------------------|------------|
| `default` | Uses default platform settings. For more details, refer to the platform-specific guide. | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | None |
| `scp03` | Device attestation provisioning example with scp03 | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03=y` |
| `user_id` | Device attestation provisioning example with user_id | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_USER_ID=y` |
| `aes_key` | Device attestation provisioning example with aes_key | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_AES_KEY=y` |
| `ec_key` | Device attestation provisioning example with ec_key | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_EC_KEY=y` |
| `scp03_user_id` | Device attestation provisioning example with scp03 + user_id | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03_USER_ID=y` |
| `scp03_aes_key` | Device attestation provisioning example with scp03 + aes key | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03_AES_KEY=y` |
| `scp03_ec_key` | Device attestation provisioning example with scp03 + ec_key | evkcmimxrt1060, frdmrw612, frdmmcxw72@cm33_core0 | Default | `-DCONFIG_SE05X_SCP03_EC_KEY=y` |
