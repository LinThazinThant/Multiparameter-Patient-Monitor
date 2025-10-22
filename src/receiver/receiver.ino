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
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);  // GPIO pin for DS18B20 data wire
DallasTemperature sensors(&oneWire);

// **FSR402 Respiratory Sensor**
const int fsrPin = 34;
const int threshold = 1000;
unsigned long lastBreathTime = 0;
int breathCount = 0;
unsigned long previousMillis = 0;
const long interval = 1000;

// **OLED Display Configuration**
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// **WiFi Credentials**
char ssid[] = "Hui";
char pass[] = "hui123456";

// **Oximeter ESP32 (Sender) IP Address**
const char* senderIP = "172.20.10.2";

// **LED Indicators**
#define LED_GREEN_TEMP 13
#define LED_RED_TEMP 12
#define LED_GREEN_SPO2 15
#define LED_RED_SPO2 23
#define LED_GREEN_BPM 25
#define LED_RED_BPM 26
#define LED_GREEN_RR 27
#define LED_RED_RR 14

// **Blynk LEDs**
WidgetLED LED1(V2);
WidgetLED LED2(V3);
WidgetLED LED3(V8);
WidgetLED LED4(V9);
WidgetLED LED5(V10);
WidgetLED LED6(V11);
WidgetLED LED7(V15);
WidgetLED LED8(V16);

BlynkTimer timer;

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("SSD1306 allocation failed!");
        while (1);
    }
    display.clearDisplay();

    sensors.begin();

    pinMode(LED_GREEN_TEMP, OUTPUT);
    pinMode(LED_RED_TEMP, OUTPUT);
    pinMode(LED_GREEN_SPO2, OUTPUT);
    pinMode(LED_RED_SPO2, OUTPUT);
    pinMode(LED_GREEN_BPM, OUTPUT);
    pinMode(LED_RED_BPM, OUTPUT);
    pinMode(LED_GREEN_RR, OUTPUT);
    pinMode(LED_RED_RR, OUTPUT);

    timer.setInterval(2000L, readAndDisplayData);
}

void loop() {
    Blynk.run();
    timer.run();
}

void readAndDisplayData() {
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    float BPM = 0, SpO2 = 0, RR = 0;

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("http://" + String(senderIP) + "/");  
        int httpCode = http.GET();  
        if (httpCode > 0) {
            String response = http.getString();
            int bpmIndex = response.indexOf("BPM");
            int spo2Index = response.indexOf("SpO2");
            if (bpmIndex != -1 && spo2Index != -1) {
                BPM = response.substring(bpmIndex + 5, spo2Index - 2).toFloat();
                SpO2 = response.substring(spo2Index + 6).toFloat();
            }
        }
        http.end();
    }

    int fsrValue = analogRead(fsrPin);
    if (fsrValue > threshold && millis() - lastBreathTime > 1000) {
        breathCount++;
        lastBreathTime = millis();
    }

    if (millis() - previousMillis >= interval) {
        previousMillis = millis();
        int RR = breathCount * 60;
        breathCount = 0;
        Blynk.virtualWrite(V14, RR);

        if (RR >= 12 && RR <= 15) {
            digitalWrite(LED_GREEN_RR, HIGH);
            digitalWrite(LED_RED_RR, LOW);
            LED7.on();
            LED8.off();
        } else {
            digitalWrite(LED_GREEN_RR, LOW);
            digitalWrite(LED_RED_RR, HIGH);
            LED7.off();
            LED8.on();
        }
    }

    String message = "Fine";
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

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
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
    display.println(message);
    display.display();
}
