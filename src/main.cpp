#include "Arduino.h"
#include "HX711.h"

// Constants
const int ledPin = LED_BUILTIN;
const long interval = 1000;

// Variables
int ledState = LOW;
unsigned long previousMillis = 0;

// HX711
HX711 loadcell;

const int LOADCELL_DOUT_PIN = D6;
const int LOADCELL_SCK_PIN = D5;

float calibration_factor = 250; 

void setup () {
    pinMode(ledPin, OUTPUT);
    Serial.begin(9600);
    Serial.println("Hello");

    loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    loadcell.set_scale();
    loadcell.tare();

    long zero_factor = loadcell.read_average();
    Serial.print("Zero factor: ");
    Serial.println(zero_factor);
}

int switchLedState(int current) {
    if (current == LOW) {
        return HIGH;
    } else {
        return LOW;
    }
}

void loop () {

    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= interval) {
        ledState = switchLedState(ledState);
        digitalWrite(ledPin, ledState);
        previousMillis = currentMillis;
    }
    loadcell.set_scale(calibration_factor); //Adjust to this calibration factor

    Serial.print("Reading: ");
    Serial.print(loadcell.get_units(), 1);
    Serial.print(" kg");
    Serial.print(" calibration_factor: ");
    Serial.print(calibration_factor);
    Serial.println();

    if(Serial.available())
    {
        char temp = Serial.read();
        if(temp == '+' || temp == 'a')
        calibration_factor += 10;
        else if(temp == '-' || temp == 'z')
        calibration_factor -= 10;
    }
}
