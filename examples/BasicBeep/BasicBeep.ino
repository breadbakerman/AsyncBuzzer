/*
  AsyncBuzzer Basic Beep Example

  This example demonstrates basic beep functionality with the AsyncBuzzer library.
  The buzzer will beep every 2 seconds in a non-blocking manner.

  Circuit:
  - Connect a piezo buzzer between pin 2 and GND
  - Or use any digital pin and update the pin number below

  Created by breadbaker, 2025
  This example code is in the public domain.
*/

#include <AsyncBuzzer.h>

const uint8_t BUZZER_OUTPUT_PIN = 2; // Pin connected to buzzer
unsigned long lastBeep = 0;
const unsigned long BEEP_INTERVAL = 2000; // Beep every 2 seconds

void setup()
{
    Serial.begin(115200);

    // Initialize the buzzer on pin 2
    if (AsyncBuzzer::setup(BUZZER_OUTPUT_PIN))
    {
        Serial.println("AsyncBuzzer initialized successfully!");
    }
    else
    {
        Serial.println("AsyncBuzzer initialization failed!");
    }

    Serial.println("Basic beep example started. Buzzer will beep every 2 seconds.");
}

void loop()
{
    // Always call update() to handle non-blocking operations
    AsyncBuzzer::update();

    // Check if it's time for another beep
    if (millis() - lastBeep >= BEEP_INTERVAL)
    {
        AsyncBuzzer::beep(); // Simple beep with default settings
        lastBeep = millis();
        Serial.println("Beep!");
    }
}