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
* **Displays:** 2 OLED (128x32)
* **Communication:** WiFi UDP, Blynk Cloud
* **Software:** ArduinoIDE, Blynk IoT Platform
* **Indicators:** 8 LEDs (4 parameters with dual-color alerts (red, green))

## System Architecture


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
**1. Install Required Libraries:**

**2. Network Setup:**
     * Update Wifi credentials in both sender and receiver code
     * Configure sender IP address in receiver code
     * Set up Blynk template with virtual pins:
       * V6: SpO2, V7: BPM, V14: Respiratory Rate
       * V2-3, V8-11, V15-16: LEDs
**3. Upload Code:**
     * Sender code to oximeter ESP32
     * Receover code to main ESP32
     * Monitor serial output at 115200 baud for connection status

## Gallery

Complete setup showing both ESP units
<img width="858" height="496" alt="image" src="https://github.com/user-attachments/assets/40a7923d-d628-43be-959e-95ce7ccf4eee" />
Wiring schematic

Real-time data on OLED screen

Mobile app interface
