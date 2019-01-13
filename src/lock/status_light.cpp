#include "status_light.h"
#include <Arduino.h>

#define YELLOW_PIN  3
#define RED_PIN     2

void status_light::setup()
{
    pinMode(YELLOW_PIN, OUTPUT);
    pinMode(RED_PIN,    OUTPUT);
}

void status_light::show_status(status_t status)
{
    Serial.println("Setting status");
    Serial.println(status);

    // show light
    switch (status) {
    case STATUS_DISENGAGED:
        digitalWrite(YELLOW_PIN,    HIGH);
        digitalWrite(RED_PIN,       LOW);
        break;

    default:
        digitalWrite(YELLOW_PIN,    LOW);
        digitalWrite(RED_PIN,       HIGH);
        break;
    }
}