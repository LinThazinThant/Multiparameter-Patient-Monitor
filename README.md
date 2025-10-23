# Multiparameter-Patient-Monitor
A comprehensive wireless health monitoring system that tracks vital signs including body temperature, pulse rate, blood oxygen saturation (SpO2), and respiratory rate. The system uses dual ESP32 microcontrollers with real-time data synchronization and remote monitoring capabilities via BLYNK mobile app.

## Features
* Real-time tracking of heart rate (bpm), oxygen saturation (SpO2), body temperature, and respiration rate
* Wireless communication between sender and receiver units using WiFi UDP protocol
* Visual LED indicators and Blynk app notifications for abnormal vital signs
* Local-real-time data visualizaion with graphic heart rate trends
* Cloud connectivity via Blynk IoT platform for mobile app access

## Hardware & Software
* **Microcontrollers:** 2 ESP32
* **Sensors:**
  * MAX30100 Pulse Oximeter (Heart Rate & SpO2)
  * DS18B20 Temperature Sensor
  * FSR402 Force Sensitive Resistor (Respiratory Rate)
* **Displays:** 2 OLED (128x64)
* **Communication:** WiFi UDP, Blynk Cloud
* **Software:** ArduinoIDE, Blynk IoT Platform
* **Indicators:** 8 LEDs (4 parameters with dual-color alerts (red, green))

## System Architecture
<img width="902" height="644" alt="image" src="https://github.com/user-attachments/assets/059ffa93-66a7-4d71-ac49-51dd725a7af6" />

## Setup & Installtion
* **Hardware Connections**
  * **Sender ESP32 (Oximeter Unit):**
     * MAX30100: I2C (SCL/ SDA)
     * OLED Display: I2C (0x3C)
  * **Receiver ESP32 (Main Unit):**
     * DS18B20: GPIO 5
     * FSR402: GPIO 34
     * OLED Display: I2C (0x3C)
     * LED Indicators: GPIO 12-15, 23, 25-27

## Software Configuration
* **1. Install Required Libraries:**
  <img width="1128" height="512" alt="image" src="https://github.com/user-attachments/assets/201c3fe3-cfd7-42f5-a854-68887aae3252" />

  * Blynk: Blynk by Volodymyr (https://github.com/blynkkk/blynk-library)
  * Pulse oximeter: MAX30100lib by OXullo (https://github.com/oxullo/Arduino-MAX30100)
  * Temperature:
    * OneWire by Jim
    * DallasTemperature by Miles (https://github.com/milesburton/Arduino-Temperature-Control-Library)
  * OLED:
    * Adafruit GFX (https://github.com/adafruit/Adafruit-GFX-Library)
    * Adafruit SSD1306 (https://github.com/adafruit/Adafruit_SSD1306)
* **2. Network Setup:**
     * Update Wifi credentials in both sender and receiver code
     * Configure sender IP address in receiver code
     * Set up Blynk template with virtual pins:
       * V6: SpO2, V7: BPM, V14: Respiratory Rate
       * V2-3, V8-11, V15-16: LEDs
* **3. Upload Code:**
     * Sender code to oximeter ESP32
     * Receover code to main ESP32
     * Monitor serial output at 115200 baud for connection status

## Gallery
* Complete setup showing both ESP units

* Wiring schematic
<img width="846" height="470" alt="image" src="https://github.com/user-attachments/assets/5f9eba5d-5a41-4c29-9b08-71efc64d321f" />
<img width="942" height="1108" alt="image" src="https://github.com/user-attachments/assets/45eab604-efff-46b6-acd0-03a4d6d3e918" />

* Real-time data on OLED screen

* Mobile app interface
<img width="1004" height="290" alt="image" src="https://github.com/user-attachments/assets/f6fd2f24-8d8d-498e-85a6-167bb3153fd1" />
<img width="940" height="274" alt="image" src="https://github.com/user-attachments/assets/73e256f2-891b-4e59-84c6-b9e81ffa15f6" />

## Technical Details
* **Data Communication Protocol**
  * Sender → Receiver: UDP packets containing "BPM, SpO2" format
  * Receiver Polling: HTTP GET requests to sender's web server
  * Cloud Integration: Blynk virtual pins for real-time app updates

## Alert Thresholds
* **Temperature:** Normal = 34-37°C
* **SpO2:** Normal ≥ 95%
* **Heart Rate:** Normal = 60-120 bpm
* **Respiratory Rate:** Normal = 12-15 breaths/minute

 ## Future Improvements

 ## Contributing
 This project demonstrates advanced embedded systems integration for medical monitoring applications. Contributions for enhanced sensor accuracy, additional health parameters, or improved user interface are welcome.
