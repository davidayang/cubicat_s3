# Cubicat S3 Device Development Framework

This framekwork is for the Cubicat S3 device, which is based on the Espressif ESP32-S3 MCU. This SDK is developed using the ESP-IDF framework and encapsulates easy-to-use hardware calling APIs, making it convenient to utilize the hardware of the Cubicat S3 device. The hardware includes a 2.4-inch LCD with touch support, a speaker, a microphone, Wi-Fi connectivity, and a virtual file system.
This SDK also includes a simple graphics engine that allows the creation of custom frame-by-frame animations and integrates a 2D physics system (Box2D). Additionally, it facilitates easy integration with LVGL, allowing for custom rendering to be combined with LVGL rendering (enabling LVGL to render above custom graphics).
We welcome any suggestions and feedback for this project.

### Cubicat S3 Menu-Configuration:

* ##### Serial flasher config

  ###### \> Flash SPI mode : QIO

  ###### \> Flash SPI speed : 80MHz

  ###### \> Flash size : 16MB
* ##### SPI RAM config

  ###### \> Mode (QUAD/OCT) of SPI RAM chip in use : Octal Mode PSRAM

  ###### \> Set RAM clock speed : 80MHz
* ##### Partition Table

  ###### \> Partition Table : Custom partition table CSV
  example:

  | #Name    | Type | SubType  | Offset   | Size    |
  | ---------- | ------ | :--------- | ---------- | --------- |
  | nvs      | data | nvs      | 0x9000   | 0x5000  |
  | otadata  | data | ota      | 0xe000   | 0x2000  |
  | app0     | app  | ota_0    | 0x10000  | 0x80000 |
  | spiffs   | data | spiffs   | 0x810000 | 0x50000 |
  | coredump | data | coredump | 0xD10000 | 0x10000 |
* ##### ESP System Settings
  ###### \> Main task stack size : 8192