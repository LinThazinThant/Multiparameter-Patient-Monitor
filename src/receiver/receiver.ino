/*****
* Multiparameter-Patient-Monitoring System - RECEIVER UNIT
* This ESP32 reads temperature data from DS18B20, repiratory rate data from FSR402, 
  and receives Heart rate & SpO2 data from sender unit via UDP
* Hardware:  ESP32, DS18B20 Temperature Sensor, FSR402 Force Sensor, SSD1306 OLED Display
* Developed by: [Lin Thazin Thant]
* Date: [2 Apr 2025]
*****/

// Blynk IoT Cloud Configuration
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPxxxxxx" // your template name here
#define BLYNK_TEMPLATE_NAME "Device"  // your project name here 
#define BLYNK_AUTH_TOKEN "YourAuthToken"  // your token here

// Library included
#include <Wire.h>   // Core hardware communication
#include <HTTPClient.h>  // HTTP requests to sender ESP32
#include <OneWire.h>  // One-wire communication for temperature sensor
#include <DallasTemperature.h>  // DS18B20 temperature sensor control
#include <Adafruit_GFX.h>       // Graphics library for OLED
#include <Adafruit_SSD1306.h>   // OLED display controller
#include <BlynkSimpleEsp32.h>  // Blynk IoT cloud connectivity
#include <WiFi.h>

// Temperature Sensor (DS18B20) Configuration
#define ONE_WIRE_BUS 5  // GPIO pin for DS18B20 data wire
OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);

// FSR402 Respiratory Sensor Configuration
const int fsrPin = 34;  // Analog pin for FSR402
const int threshold = 1000;  // Minimum analog value
unsigned long lastBreathTime = 0;  // Time of last detected breath
int breathCount = 0;  // for current interval
unsigned long previousMillis = 0;  // timer for respiratory rate calculation
const long interval = 1000;  // one second interval

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi Credentials
const char* ssid = "xxx"; // your wifi name here
const char* password = "xxx"; // your wifi password here
const char* senderIP = "xxx.xx.xx.x";  // your sender ESP32's IP here

// LED Indicators Configuration
#define LED_GREEN_TEMP 13  // GPIO for normal temperature (green)
#define LED_RED_TEMP 12    // GPIO for abnormal temperature (red)
#define LED_GREEN_SPO2 15  // GPIO for normal SpO2 level (green)
#define LED_RED_SPO2 23    // GPIO for abnormal SpO2 level (red)
#define LED_GREEN_BPM 25   // GPIO for normal heart rate (green)
#define LED_RED_BPM 26    // GPIO for abnormal heart rate (red)
#define LED_GREEN_RR 27   // GPIO for normal respiratory rate (green)
#define LED_RED_RR 14     // GPIO for abnormal respiratory rate (red)

// Blynk LEDs Indicators
WidgetLED LED1(V2);  // Blynk virtual LED for temprature (green)
WidgetLED LED2(V3);  // Blynk virtual LED for temprature (red)
WidgetLED LED3(V8);  // Blynk virtual LED for SpO2 (green)
WidgetLED LED4(V9);  // Blynk virtual LED for SpO2 (red)
WidgetLED LED5(V10); // Blynk virtual LED for heart rate (green)
WidgetLED LED6(V11); // Blynk virtual LED for heart rate (red)
WidgetLED LED7(V15); // Blynk virtual LED for respiratory rate (green)
WidgetLED LED8(V16); // Blynk virtual LED for respiratory rate (red)

BlynkTimer timer;  // Timer for periodic data reading

/*****
* SETUP FUNCTION - Initializes all components when ESP32 starts
* This function runs once at startup and prepares all hardware/ software components:
  - Serial communication
  - Wifi and cloud connectivity
  - Web server for API access
  - OLED display for local monitoring
  - Temperature sensor and force sensor for health data
*****/
void setup() {
    // Initialize serial communication (115200 baud rate)
    Serial.begin(115200);

    //  Initialize Blynk IoT Cloud connection
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

    //  WiFi connection for web sever functionality
    WiFi.begin(ssid, password);

     // Wait for WiFi connection with visual progress indicators
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");

    // Initialize OLED Display (128x64 pixels)
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("SSD1306 allocation failed!");
        while (1);
    }
    display.clearDisplay();

    // Initialize temperature sensor
    sensors.begin();

    // Configure physical LED pins as outputs
    pinMode(LED_GREEN_TEMP, OUTPUT);
    pinMode(LED_RED_TEMP, OUTPUT);
    pinMode(LED_GREEN_SPO2, OUTPUT);
    pinMode(LED_RED_SPO2, OUTPUT);
    pinMode(LED_GREEN_BPM, OUTPUT);
    pinMode(LED_RED_BPM, OUTPUT);
    pinMode(LED_GREEN_RR, OUTPUT);
    pinMode(LED_RED_RR, OUTPUT);

    timer.setInterval(2000L, readAndDisplayData);  // Set up timer to read and display data every 2 seconds
}

/*****
* MAIN LOOP - Handles Blynk communication and timer events
*****/
void loop() {
    Blynk.run();  // Maintain Blynk cloud connection
    timer.run();  // Execute timed funcions (readAndDisplayData)
}

/*****
* DATA PROCESSING FUNCTION - Reads all sensors and update displays
* This function is called every 2 seconds by BlynkTimer
*****/
void readAndDisplayData() {
    // Read temperature from DS18B20 sensor
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
  
    float BPM = 0, SpO2 = 0, RR = 0;   // Variables to store data received from sender ESP32

    // Get heart rate and SpO2 from Sender ESP32 via HTTP
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("http://" + String(senderIP) + "/");  // Create HTTP request to Sender's web API
        int httpCode = http.GET();  

        // Process response if request is successful
        if (httpCode > 0) {
            String response = http.getString();  // get response {BPM, SpO2)
            int bpmIndex = response.indexOf("BPM");
            int spo2Index = response.indexOf("SpO2");
            if (bpmIndex != -1 && spo2Index != -1) {
                BPM = response.substring(bpmIndex + 5, spo2Index - 2).toFloat();
                SpO2 = response.substring(spo2Index + 6).toFloat();
            }
        }
        http.end();  // free resources
    }

    // Calculate respiratory rate using FSR402 sensor
    int fsrValue = analogRead(fsrPin);  // real analog pin value from force sensor
    
    //Detect breath
    if (fsrValue > threshold && millis() - lastBreathTime > 1000) {
        breathCount++;
        lastBreathTime = millis();
    }

    // Calculate respiratory rate every second
    if (millis() - previousMillis >= interval) {
        previousMillis = millis();
        int RR = breathCount * 60;  // Convert breaths per interval to breaths per minute
        breathCount = 0;  // Reset counter for next interval
        Blynk.virtualWrite(V14, RR);  // Send respiratory rate to Blynk app

        String message = "Fine";
        
        // RR alert 
        if (RR >= 12 && RR <= 15) {  // Normal RR
            digitalWrite(LED_GREEN_RR, HIGH);  // green LED on
            digitalWrite(LED_RED_RR, LOW);    // red LED off
            LED7.on();  // Blynk green LED on
            LED8.off(); // Blynk red LED off
        } else {  // Abnromal RR
            digitalWrite(LED_GREEN_RR, LOW);  // green LED off
            digitalWrite(LED_RED_RR, HIGH);  // red LED on
            LED7.off();  // Blynk green LED off
            LED8.on();  // Blynk red LED on
            message = "Shortness of breath";
        }
    }

        // Temperature alert (Normal range: 34-37°C)
        if (temperatureC < 34 || temperatureC > 37) {
            digitalWrite(LED_GREEN_TEMP, LOW);
            digitalWrite(LED_RED_TEMP, HIGH);
            LED1.off();
            LED2.on();
            message = "Take treatment";
        } else {
            digitalWrite(LED_GREEN_TEMP, HIGH);
            digitalWrite(LED_RED_TEMP, LOW);
            LED1.on();
            LED2.off();
        }

        // SpO2 alert (Normal: ≥95%)
        if (SpO2 < 95) {
            digitalWrite(LED_GREEN_SPO2, LOW);
            digitalWrite(LED_RED_SPO2, HIGH);
            LED3.off();
            LED4.on();
            message = "Oxygen required";
        } else {
            digitalWrite(LED_GREEN_SPO2, HIGH);
            digitalWrite(LED_RED_SPO2, LOW);
            LED3.on();
            LED4.off();
        }

        // HR alert (Normal: 60-120 bpm)
        if (BPM < 60 || BPM > 120) {
            digitalWrite(LED_GREEN_BPM, LOW);
            digitalWrite(LED_RED_BPM, HIGH);
            LED5.off();
            LED6.on();
            message = "Take treatment";
        } else {
            digitalWrite(LED_GREEN_BPM, HIGH);
            digitalWrite(LED_RED_BPM, LOW);
            LED5.on();
            LED6.off();
        }

    // Update OLED display with current readings
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    // Display all vital signs
    display.print("Temp: "); 
    display.print(temperatureC);
    display.println(" C");
    display.print("Pulse: "); 
    display.print(BPM);
    display.println(" bpm");
    display.print("SpO2: "); 
    display.print(SpO2);
    display.println(" %");
    display.print("RR: "); 
    display.print(RR); 
    display.println(" bpm");
    display.println(message);  // Show alert message
    display.display();  // Update physical display
}
