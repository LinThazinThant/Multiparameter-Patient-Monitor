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
* * MAX30100 Pulse Oximeter (Heart Rate & SpO2)
  * DS18B20 Temperature Sensor
  * FSR402 Force Sensitive Resistor (Respiratory Rate)
* **Displays:** 2 OLED
* **Communication:** WiFi UDP, Blynk Cloud
* **Software:** ArduinoIDE, Blynk IoT Platform
* **Indicators:** 8 LEDs (4 parameters with dual-color alerts (red, green))
