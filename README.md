# SoundDancing

An Arduino-based project designed to create a sound-reactive and light-reactive to control remotely Chrono Lumina through sound sensor, LDR, NeoPixel and MQTT.     
This project is the Assignment for **CASA0014 Connected Environments**. 
<div align="center">
<img src="./ReadMeSrc/img/37f52d12dba750406626fa2875f9a6ca.jpg" alt="11" width="150" height="150">
</div>

## Project Status 🚧

This project has finished the prototype. Major features has being implemented, and still will improve in the future.
<div align="center">
<img src="./ReadMeSrc/img/1aeaeb81a5c16ff70a46be5764e9bd0.jpg" alt="prototype" width="500" height="500">
</div>

---

## Table of Contents 📂

- [Background and Introduction](#background-and-introduction-)
- [Features](#features-)
- [Hardware and Components](#hardware-and-components-)
- [Software and Libraries](#software-and-libraries-)
- [Progress and Roadmap](#progress-and-roadmap-)
- [License](#license-)

---

## Background and Introduction 🌟
<div align="center">
    <img src="./ReadMeSrc/img/light.jpg" alt="Light" width="600" height="600">
</div>
 
   
Welcome to the **SoundDancing** project! This idea is inspired by my own rhythm lights and my strong interest in the fusion of sound and light and shadow. In my previous projects, I designed a smart devices based on environmental data, the learning assistants that monitor the learning environment and sound playback systems. This made me realize that sound can not only be heard, but can also be "seen" in a more intuitive way. So, I would like to develop the soundDancing to achieve the sound visualization.   

Through the spectral analysis and rhythm extraction of environmental sounds, the **SoundDancing** is driven by an optimized algorithm to present diverse lighting patterns, enabling it to adapt to different sound scenarios (such as music, ambient sounds, or speech). This project not only embodies the cross-domain connection between sound and vision but also offers novel ideas for the research and design of interactive intelligent devices.

---

## Features ✨

- [x] **Light Monitoring**: Detect environmental light levels using a light sensor to adjust the brightness of Chrono Lumina Light.
- [x] **Sound Detection**: Detect environmental sound using a microphone sensor to collect sound data, and using these data to control remotely the Chrono Lumina Light.
- [x] **OLED Screen Display**: Display the option to the user to select the color mode (Single Mode and Random Color Mode).
- [x] **Wi-Fi Connectivity**: Transmit data remotely using the MKR 1010 WiFi board, control the Chrono Lumina Light remotely.
- [x] **Sound Processing**: Find a way to avoid the noise and other elements, which will influence the sound data.
- [x] **User Alerts**: Provide a rotary encoder to let user can choose the color mode on the screen.

---

## Hardware and Components 🔧

- **Arduino Board**: Arduino MKR 1010 WiFi
- **Sensors**: Light sensor, microphone sensor
- **Display**: OLED Screen
- **Actuators**: Rotary encoder

---

## Software and Libraries 🛠

- **IDE**: 
    - `Arduino IDE`
- **Libraries**: 
    - `Adafruit_GFX` 
    - `WiFiNINA`
    - `Wire`
    - `Adafruit_SSD1306`
    - `WiFiNINA`
    - `PubSubClient`
---

## Progress and Roadmap 🗓

### Progress:
- [x] Light sensor test
- [x] Sound sensor test
- [x] OLED screen test
- [x] MQTT connection test and public messages to topic test
- [x] Rotary encoder test
- [x] Combine the OLED screen and rotary encoder together to achieve the selection function
- [x] Combine the Light sensor and Sound sensor to control the Chrono Lumina Light remotely and released the first version of project. Control the one Chrono Lumina Light.
- [x] During the test processing, found the noise will influence the sound data collection. It need to a way to avoid this issue. Use the sample windows to calculate the average data  (on software part) 
- [x] Combine the OLED screen and Rotary encoder to the main project  
- [x] Optimized encloser 
#### Current status

##### Circuit Diagram
<div align="center">
<img src="./ReadMeSrc/img/cirult.png" alt="circuit" width="500" height="500">
</div>

##### Work Flow
<div align="center">
<img src="./ReadMeSrc/img/workflow.png" alt="circuit">
</div>

- OLED Screen and Rotary encoder
     - User can select like color
        - Red
        - Blue
        - Green
        - Yellow
        - white
        - Purple

-  Sound Sensor     
     - Based on the sound level from sound sensor to turn on different number led of neopixel 
        - sound level (30-150 db) map to leds number (0 - 12)
-  LDR
    - Based on the light level from LDR to adjust the brightness of the LED
        - light level (0 - 1023) map to brightness value(0 - 120)
### Demonstration:
[Video](https://github.com/mk20661/CASA0014_Coursework/blob/main/ReadMeSrc/ce.mp4)
<video width="400" controls autoplay>
    <source src="./ReadMeSrc/ce.mp4" type="video/mp4">
</video>

### Future Plan:

- [ ] Develop the filter, such as the low pass filter, and using FFT to ensure the sound data clean
- [ ] Control more Chrono Lumina Lights, not only one
- [ ] Future updating

---

## License 📜
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)   
This project is licensed under the [MIT License](LICENSE).
