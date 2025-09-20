/*
  AsyncBuzzer Pulse Pattern Example

  This example demonstrates pulse pattern functionality with the AsyncBuzzer library.
  The buzzer will play different pulse patterns in sequence.

  Circuit:
  - Connect a piezo buzzer between pin 2 and GND
  - Or use any digital pin and update the pin number below

  Created by breadbaker, 2025
  This example code is in the public domain.
*/

#include <AsyncBuzzer.h>

const uint8_t BUZZER_OUTPUT_PIN = 2; // Pin connected to buzzer

void setup()
{
    Serial.begin(115200);

    // Initialize the buzzer
    if (AsyncBuzzer::setup(BUZZER_OUTPUT_PIN))
    {
        Serial.println("AsyncBuzzer initialized successfully!");
    }
    else
    {
        Serial.println("AsyncBuzzer initialization failed!");
    }

    Serial.println("Pulse pattern example started.");
    Serial.println("Press 'a' for short pulses, 'b' for long pulses, 'c' for quick pulses");
}

void loop()
{
    // Always call update() to handle non-blocking operations
    AsyncBuzzer::update();

    // Check for serial input
    if (Serial.available())
    {
        char input = Serial.read();

        switch (input)
        {
        case 'a':
        case 'A':
            Serial.println("Playing short pulses");
            AsyncBuzzer::pulse(5, 100, 200); // 5 pulses, 100ms on, 200ms off
            break;

        case 'b':
        case 'B':
            Serial.println("Playing long pulses");
            AsyncBuzzer::pulse(3, 300, 400); // 3 pulses, 300ms on, 400ms off
            break;

        case 'c':
        case 'C':
            Serial.println("Playing quick pulses");
            AsyncBuzzer::pulse(10, 50, 50); // 10 pulses, 50ms on, 50ms off
            break;

        default:
            Serial.println("Unknown command. Use 'a', 'b', or 'c'");
            break;
        }
    }
}