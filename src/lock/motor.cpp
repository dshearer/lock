#include <Arduino.h>
#include "motor.h"

#define ENGAGE_PIN      7
#define DISENGAGE_PIN   6

namespace motor {

void setup() {
    pinMode(ENGAGE_PIN, OUTPUT);
    pinMode(DISENGAGE_PIN, OUTPUT);
    stop();
}

void runEngage() {
    digitalWrite(ENGAGE_PIN, HIGH);
    digitalWrite(DISENGAGE_PIN, LOW);
}

void runDisengage() {
    digitalWrite(ENGAGE_PIN, LOW);
    digitalWrite(DISENGAGE_PIN, HIGH);
}

void stop() {
    digitalWrite(ENGAGE_PIN, LOW);
    digitalWrite(DISENGAGE_PIN, LOW);
}

}