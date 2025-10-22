/*****
* Multiparameter-Patient-Monitoring System - SENDER UNIT
* This ESP32 reads pulse oximetry (Heart rate, SpO2) data from MAX30100 sensor 
  and transfers it wirelessly to the receiver unit via UDP.
* Hardware: ESP32, MAX30100 Pulse Oximeter, SSD1306 OLED Display
* Developed by: [Lin Thazin Thant]
* Date: [2 Apr 2025]    
*****/

// Blynk IoT Cloud Configuration
#define BLYNK_TEMPLATE_ID "TMPxxxxxx" // your template name here
#define BLYNK_TEMPLATE_NAME "Device"  // your project name here 
#define BLYNK_AUTH_TOKEN "YourAuthToken"  // your token here

// Library included
#include <Wire.h>   // Core hardware communication
#include "MAX30100_PulseOximeter.h" // Pulse oximeter sensor
#define BLYNK_PRINT Serial      
#include <BlynkSimpleEsp32.h>   // Blynk IoT platform
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiUdp.h>    // WiFi & Network libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>   // Display libraries

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi Credentials
const char* ssid = "xxx"; // your wifi name here
const char* password = "xxx"; // your wifi password here
WebServer server(80);
WiFiUDP udp;
const char* receiverIP = "xxx.xx.xx.x"; // your receiver ESP32's IP here
const int receiverPort = 1234;

// Pulse Oximeter and Variables
PulseOximeter pox;
float BPM = 0.0;  // Beats per minute for heart rate
float SpO2 = 0.0; // Blood oxygen saturation %
uint32_t tsLastReport = 0;
#define REPORTING_PERIOD_MS 1000  // transmit data every 1 second

// Graph Data
#define GRAPH_WIDTH 128
#define GRAPH_HEIGHT 50
int graph[GRAPH_WIDTH] = {0};
int graphIndex = 0;

// Heartbeat Detection Callback - triggered when pulse is detected
void onBeatDetected() {
    Serial.println("Heart Beat Detected!");
}

// Web Server Response Handler
void handleRoot() {
    String jsonResponse = "{\"BPM\":" + String(BPM) + ", \"SpO2\":" + String(SpO2) + "}"; // current health data
    server.send(200, "application/json", jsonResponse); // send to response to receiver ESP32
}

/*****
* SETUP FUNCTION - Initializes all components when ESP32 starts
* This function runs once at startup and prepares all hardware/ software components:
  - Serial communication
  - Wifi and cloud connectivity
  - Web server for API access
  - OLED display for local monitoring
  - Pulse oximeter sensor for health data
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
    Serial.print("ESP32 Oximeter IP Address: ");
    Serial.println(WiFi.localIP()); // Display IP for network access

    //  Configure web server routes and start server
    server.on("/", handleRoot);
    server.begin();

    //  Initialize UDP communication for real-time data streaming
    udp.begin(receiverPort);

    // Initialize OLED Display (128x64 pixels)
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (true);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Initializing...");
    display.display();

    // Initialize MAX30100 Pulse Oximeter Sensor
    Serial.print("Initializing Pulse Oximeter...");
    if (!pox.begin()) {
        Serial.println("FAILED");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Oximeter Failed"); // check the sensor if this appears
        display.display();
        while (true);
    } else {
        Serial.println("SUCCESS");
        pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
        pox.setOnBeatDetectedCallback(onBeatDetected);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Oximeter Ready");
        display.display();
    }
}

/*****
* MAIN LOOP - Continuous operation and data processing
* This function runs repeatedly and handles:
  - Sensor data acquisition and processing
  - Multi-channel data transmission (Blynk cloud, UDP, local display)
  - Real-time data validattion and visualization
  - Heart rate graph rendering on OLED display
*****/
void loop() {
    //  Update pulse oximeter sensor readings
    pox.update();

    //  Handle Blynk cloud communication and keep connection alive
    Blynk.run();

    // API calls
    server.handleClient();

    // Read current heaalth metrics from sensor
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();

    // Data transmission block
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        if (BPM < 30 || BPM > 220) BPM = 0; // Normal human heart rate: 30-220 bpm
        if (SpO2 < 70 || SpO2 > 100) SpO2 = 0;  // Normal SpO2 range: 70-100%

        // Update heart rate graph data
        graph[graphIndex] = BPM;
        graphIndex = (graphIndex + 1) % GRAPH_WIDTH;

        // Serial output - for monitoring
        Serial.print("Pulse rate: ");
        Serial.print(BPM);
        Serial.print(" bpm / SpO2: ");
        Serial.print(SpO2);
        Serial.println(" %");

        // Blynk cloud transmission - send to mobile app
        Blynk.virtualWrite(V7, BPM);  // Virtual Pin 7 = Heart Rate
        Blynk.virtualWrite(V6, SpO2); // Virtual Pin 6 = Oxygen Saturation

        // UDP transmission - real-time data streaming to receiver ESP32
        String message = String(BPM) + "," + String(SpO2);
        udp.beginPacket(receiverIP, receiverPort);
        udp.print(message);
        udp.endPacket();

        // OLED display update - local real-time visualization
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        
        // Heart rate graph rendering
        /* 
          * Creates a real-time scrolling graph of heart rate data
          * X-axis: Time
          * Y-axis: Heart rate (BPM values)
        */
        int graphOffset = 5;  // Pixel offset from bottom of screen

        // Draw baseline for graph reference
        display.drawLine(0, SCREEN_HEIGHT - GRAPH_HEIGHT - graphOffset, SCREEN_WIDTH, SCREEN_HEIGHT - GRAPH_HEIGHT - graphOffset, SSD1306_WHITE);
        // Draw line graoh connecting consecutive data points
        for (int i = 0; i < GRAPH_WIDTH - 1; i++) {
            int x1 = i; // Current position
            int y1 = SCREEN_HEIGHT - graph[i] / 2 - graphOffset;
            int x2 = i + 1; // Next position
            int y2 = SCREEN_HEIGHT - graph[i + 1] / 2 - graphOffset;
            display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);  // Draw line between current and next data point
        }
        display.display();
        tsLastReport = millis();  // Update timestamp for next reporting interval
    }
}
