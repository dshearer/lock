#include "status_light.h"
#include <Arduino.h>

#define RED_PIN     13
#define GREEN_PIN   12
#define BLUE_PIN    11

#define DISPLAY_TIME_MS 1000

void status_light::init()
{
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
}

void status_light::show_status(status_t status)
{
    // show light
    switch (status) {
    case STATUS_SUCCESS:
        digitalWrite(GREEN_PIN, HIGH);
        digitalWrite(RED_PIN, LOW);
        break;

    default:
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(RED_PIN, HIGH);
        break;
    }

    // wait
    delay(DISPLAY_TIME_MS);

    // turn light off
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
}