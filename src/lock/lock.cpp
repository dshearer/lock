#include "lock.h"
#include <Arduino.h>
#include <Servo.h>

#define SERVO_PIN 7

static Servo g_servo;

static void print_pos()
{
    Serial.print("Position: ");
    Serial.print(g_servo.read());
    Serial.print("\n");
}

void lock::setup() 
{
    g_servo.attach(SERVO_PIN);
    print_pos();
}

void lock::set_lock_state(lock_state_t new_state)
{
    switch (new_state) {
    case LOCK_ENGAGED:
        g_servo.write(0);
        print_pos();
        break;

    default:
        g_servo.write(180);
        print_pos();
        break;
    }
    delay(1000);
}