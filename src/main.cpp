#include "Arduino.h"
#include "HX711.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// Constants
const int ledPin = LED_BUILTIN;
const long interval = 1000;
const long httpInterval = 500;

const String UPLOAD_PATH = "http://192.168.0.9:8000/device/1";

// Variables
int ledState = LOW;
unsigned long previousMillis = 0;

// HX711
HX711 loadcell;

const int LOADCELL_DOUT_PIN = D6;
const int LOADCELL_SCK_PIN = D5;

float calibration_factor = 25; 



void setup () {
    pinMode(ledPin, OUTPUT);
    Serial.begin(9600);

    loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    loadcell.set_scale(calibration_factor); //Adjust to this calibration factor
    loadcell.tare();

    WiFi.begin("comhem_D4E240", "7f8c6pvt");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Waiting for connection...");
    }
}

int switchLedState(int current) {
    if (current == LOW) {
        return HIGH;
    } else {
        return LOW;
    }
}

void createMeasurement(String& output, double reading) {
    int arrayLength = 1;
    
    const size_t capacity = JSON_ARRAY_SIZE(arrayLength) + JSON_OBJECT_SIZE(1) + arrayLength * JSON_OBJECT_SIZE(2);
    DynamicJsonDocument doc(capacity);

    JsonArray readings = doc.createNestedArray("readings");

    JsonObject readings_0 = readings.createNestedObject();
    readings_0["reading"] = reading;
    readings_0["timedelta"] = 0;

    serializeJson(doc, output);
}

void loop () {
    unsigned long currentMillis = millis();
    

    // ----------------------
    // HTTP
    // ----------------------
    

    if (currentMillis - previousMillis >= httpInterval) {
        String payload;
        float reading = loadcell.get_units(10);
        createMeasurement(payload, reading);
        Serial.println(payload);

        Serial.println("Starting http request");
        if (WiFi.status() == WL_CONNECTED) {
            WiFiClient client;
            HTTPClient http;
            http.begin(client, UPLOAD_PATH);
            http.addHeader("Content-Type", "application/json");

            int httpCode = http.POST(payload);

            if (httpCode > 0) {
                String payload = http.getString();
                Serial.println(payload);
            } else {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }

            http.end();
        }
        previousMillis = currentMillis;
    }

    // ----------------------
    // Readings
    // ----------------------
    
    // if (currentMillis - previousMillis >= interval) {
    //     ledState = switchLedState(ledState);
    //     digitalWrite(ledPin, ledState);
    //     previousMillis = currentMillis;
    // }
    // loadcell.set_scale(calibration_factor); //Adjust to this calibration factor

    // Serial.print("Reading: ");
    // Serial.print(loadcell.get_units(), 1);
    // Serial.print(" kg");
    // Serial.print(" calibration_factor: ");
    // Serial.print(calibration_factor);
    // Serial.println();

    // if(Serial.available())
    // {
    //     char temp = Serial.read();
    //     if(temp == '+' || temp == 'a')
    //     calibration_factor += 10;
    //     else if(temp == '-' || temp == 'z')
    //     calibration_factor -= 10;
    // }
}
