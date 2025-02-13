# Bluetooth Keyboard to USB HID Proxy

**Dyngl** is an overly complex solution to a simple problem: how to use a Bluetooth keyboard to control the bootloader/UEFI.

## Requirements

You will need two ESP32-based boards, because who needs support for both Bluetooth Classic and USB OTG on the same chip anyway?

### Hardware:
- **Bluetooth Classic board** (e.g., ESP32-DevKitC-v4)
- **USB OTG board** (e.g., ESP32-S3-DevKitC-1)
- **4 jumper wires** for SPI connection

### Software:
- **ESP-IDF** installed and set up (tested with ESP-IDF v5.4)

## Configuration

### SPI

The ESP32 acts as a Bluetooth host, receiving keyboard reports and transmitting them to the ESP32-S3 via SPI.

Example pin configuration for **ESP32-DevKitC-v4** and **ESP32S3-DevKitC-1**:

| Function | ESP32 Pin | ESP32S3 Pin |
|----------|----------|------------|
| MISO     | GPIO19   | GPIO13     |
| MOSI     | GPIO23   | GPIO11     |
| SCLK     | GPIO18   | GPIO12     |
| CS       | GPIO5    | GPIO10     |

You can change these pin assignments in the following files:
- **ESP32 (dyngl_bt)**: `dyngl_bt/main/dyngl_spi.c`, function `dyngl_spi_init()`
- **ESP32S3 (dyngl_usb)**: `dyngl_usb/main/dyngl_spi.c`, function `dyngl_spi_init()`

### Keyboard MAC Address

`dyngl_bt` needs to know your Bluetooth keyboard's MAC address. You can configure it using **idf.py menuconfig**:

```sh
cd dyngl_bt
idf.py menuconfig
```

Then navigate to:
```
DYNGL Settings → Target Device MAC Address
```

## Build and Flash

### Flashing the Bluetooth Host (ESP32)

Connect your **ESP32**, then run:
```sh
cd dyngl_bt
idf.py set-target esp32
idf.py --port <ESP32_PORT> flash
```

### Flashing the USB HID Device (ESP32S3)

Connect your **ESP32S3**, then run:
```sh
cd dyngl_usb
idf.py set-target esp32s3
idf.py --port <ESP32S3_PORT> flash
```

## Usage

1. Power on both ESP32 and ESP32S3.
2. Put your Bluetooth keyboard into pairing mode.
3. The ESP32 will connect to the keyboard and start forwarding keystrokes to the ESP32S3.
4. The ESP32S3 will act as a USB keyboard and send keystrokes to the computer.

Enjoy using your Bluetooth keyboard in the UEFI/bootloader!

