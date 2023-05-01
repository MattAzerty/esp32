# Firmwares for the microcontroller ESP32 (C++)
Board: Helteck WiFi LoRa 32(V2 & V3)

## About ESP32
The ESP32 is a low-cost, low-power, Wi-Fi and Bluetooth enabled microcontroller that is commonly used in a variety of Internet of Things (IoT) applications.

Some of the key features of the ESP32 include:

- Dual-core Tensilica LX6 processor with clock speeds up to 240 MHz
- Integrated Wi-Fi and Bluetooth radios with support for a wide range of protocols
- Multiple GPIO pins and other hardware interfaces for connecting to various sensors, actuators, and other devices
- Low power consumption and deep sleep modes, making it ideal for battery-powered applications

## RoomSystem_ambiantSystem_ESP32HELTEC.ino
<img src="https://user-images.githubusercontent.com/21226834/235457033-428f8851-c87e-459a-891a-dc784a094a4d.jpg" width="500">

This diriver made is part of the project [RoomSytem](https://github.com/MattAzerty/RoomSystem)

Some Feature:
- If no Wi-Fi credentials are stored, Bluetooth (Classic) will be activated and the device will wait for configuration through a phone.
- The Firebase User ID provided by the phone is used to store temperature and humidity readings every 6 hours.
- Thanks to the library [Firebase-ESP32](https://github.com/mobizt/Firebase-ESP32) library, the microchip is able to receive real-time notifications (using HTTP streaming) when the user is connected to the app.
- When the user is connected, the sensor readings are updated in real-time instead of only every 6 hours.

## BioAmp_EXG_Pill_EEG_recording_ESP32.ino
![PXL_20230414_160534009](https://user-images.githubusercontent.com/21226834/235460151-26c4f648-89e7-479d-8dcf-a0b71a80d0a8.jpg | width=100)
The goal of this side project is to record brainwave frequencies (alpha and beta waves) through EEG measurements.
With the help of an ESP32 and [an analog front-end amplification](https://github.com/upsidedownlabs/BioAmp-EXG-Pill/blob/main/software/EEGFilter/EEGFilter.ino) made by the upsidedownlabs Company.
