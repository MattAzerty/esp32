# Firmwares for the microcontroller ESP32 (C++)
Board: Helteck WiFi LoRa 32(V2 & V3)

## About ESP32
<img src="https://user-images.githubusercontent.com/21226834/235464446-6779fa98-fdd9-4564-8f1c-ca7764851655.png" width="500">

The [ESP32](https://www.espressif.com/en/products/socs/esp32) is a low-cost, low-power, Wi-Fi and Bluetooth enabled microcontroller that is commonly used in a variety of Internet of Things (IoT) applications.

Some of the key features of the ESP32 include:

- Dual-core Tensilica LX6 processor with clock speeds up to 240 MHz
- Integrated Wi-Fi and Bluetooth radios with support for a wide range of protocols
- Multiple GPIO pins and other hardware interfaces for connecting to various sensors, actuators, and other devices
- Low power consumption and deep sleep modes, making it ideal for battery-powered applications

## RoomSystem_ambiantSystem_ESP32HELTEC.ino
<img src="https://user-images.githubusercontent.com/21226834/235457033-428f8851-c87e-459a-891a-dc784a094a4d.jpg" width="500">

I have developed this driver for the [RoomSytem](https://github.com/MattAzerty/RoomSystem) project, which was created to experiment with Kotlin and Android.

Some Feature:
- If no Wi-Fi credentials are stored, Bluetooth (Classic) is activated and the device will wait for configuration through a phone.
- The Firebase UserID provided by the phone at initial configuration is used to store temperature and humidity readings every 6 hours.
- Thanks to the library [Firebase-ESP32](https://github.com/mobizt/Firebase-ESP32), the microchip is able to receive real-time notifications (using HTTP streaming) when the user is connected to the app.
- Sensor readings are updated in real-time on Firebase instead of only every 6 hours when the user is connected.

## BioAmp_EXG_Pill_EEG_recording_ESP32.ino
<img src="https://user-images.githubusercontent.com/21226834/235460151-26c4f648-89e7-479d-8dcf-a0b71a80d0a8.jpg" height="300">
-Electrode test recording on muscle Arm (easier to detect)

The final goal of this side project is to record brainwave frequencies (alpha and beta waves) through EEG measurements.
With the help of an ESP32 and an [analog front-end amplification](https://github.com/upsidedownlabs/BioAmp-EXG-Pill/blob/main/software/EEGFilter/EEGFilter.ino) made by the upsidedownlabs Company.
<img src="https://user-images.githubusercontent.com/21226834/235469761-248f4fae-6f3b-4e28-9edc-3314f5b365a8.png" width="500">
-Test UI ideas.
