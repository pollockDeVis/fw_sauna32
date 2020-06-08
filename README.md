# DF_Entropy

Bluetooth enabled temperature sensor with display and proximity sensing



## Encryption

Navigate to `C:\Users\Pollock\Desktop\esp-idf\components\esptool_py\esptool`

Generate Key

```
espsecure.py generate_flash_encryption_key C:/Users/Pollock/Documents/Repo/Sauna32/entropy/EntropyEncryptionKey.bin
```

Burn Key

```
espefuse.py --port COM24 burn_key flash_encryption C:/Users/Pollock/Documents/Repo/Sauna32/entropy/EntropyEncryptionKey.bin
```

`Burned key data. New value: e9 51 15 54 33 4d 80 15 0c f2 e5 ad c7 d9 18 52 e9 44 61 57 87 50 f1 18 3d 4d 4f 8f f7 27 f3 80`

Turn Encryption on in menuconfig and set it to development mode

Run `idf.py flash monitor ` for the first time

Subsequent plain-text flash must be done using `idf.py encrypted-app-flash monitor` or `idf.py encrypted-flash monitor` 

