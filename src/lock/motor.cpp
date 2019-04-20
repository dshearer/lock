#include <Arduino.h>
#include "motor.h"

#define LEFT_PIN    6
#define RIGHT_PIN   7

namespace motor {

void setup() {
    pinMode(LEFT_PIN, OUTPUT);
    pinMode(RIGHT_PIN, OUTPUT);
}

void runLeft() {
    digitalWrite(LEFT_PIN, HIGH);
    digitalWrite(RIGHT_PIN, LOW);
}

void runRight() {
    digitalWrite(LEFT_PIN, LOW);
    digitalWrite(RIGHT_PIN, HIGH);
}

void stop() {
    digitalWrite(LEFT_PIN, LOW);
    digitalWrite(RIGHT_PIN, LOW);
}

}