#include <Arduino.h>
#include <utils.h>
#include "status_sensor.h"

#define ENGAGED_SENSOR_PIN      2
#define DISENGAGED_SENSOR_PIN   3

#define BTN_PRESSED_LEVEL             LOW
#define BTN_PRESSED_INTERRUPT_LEVEL   LOW
#define BTN_RELEASED_INTERRUPT_LEVEL  RISING

namespace status_sensor {

/*
There are two buttons, one of which is depressed when the lock is engaged
and one of which is depressed when the lock is disengaged.

It is not possible for both to be depressed, but it is possible for
neither of them to be.  In this case, the lock is neither engaged nor
disengaged, and its effectiveness at keeping the door closed is unknown.
*/

/* State: */
// static callback_t g_callbacks[8] = {0};
// static size_t g_nbrCallbacks = 0;

// static void handleStatusChange()
// {
//     const status_t status = getStatus();
//     for (size_t i = 0; i < g_nbrCallbacks; ++i) {
//         g_callbacks[i](oldStatus, g_status);
//     }
// }

void setup()
{
    // enable internal pullup resistors for button pins
    pinMode(ENGAGED_SENSOR_PIN,     INPUT_PULLUP);
    pinMode(DISENGAGED_SENSOR_PIN,  INPUT_PULLUP);

    // attach interrupts
    // attachInterrupt(digitalPinToInterrupt(ENGAGED_SENSOR_PIN),
    //     handleStatusChange, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(DISENGAGED_SENSOR_PIN), 
    //     handleStatusChange, CHANGE);
}

// void addCallback(callback_t cb)
// {
//     if (g_nbrCallbacks >= NUM_ELEMS(g_callbacks)) {
//         return;
//     }

//     g_callbacks[g_nbrCallbacks] = cb;
//     ++g_nbrCallbacks;
// }

status_t getStatus()
{
    /*
    There is a (very unlikely) race-condition here, where both
    buttons seem to be depressed.  In this case, we just
    read them again after a short wait.
    */

    const bool engaged = digitalRead(ENGAGED_SENSOR_PIN) == BTN_PRESSED_LEVEL;
    const bool disengaged = digitalRead(DISENGAGED_SENSOR_PIN) == BTN_PRESSED_LEVEL;

    bool tryingAgain = false;
    status_t status = STATUS_UNKNOWN;

tryAgain:
    if (engaged && disengaged) {
        if (!tryingAgain) {
            delay(10);
            tryingAgain = true;
            goto tryAgain;
        }
        else {
            status = STATUS_UNKNOWN;
        }
    }
    else if (engaged) {
        status = STATUS_ENGAGED;
    }
    else if (disengaged) {
        status = STATUS_DISENGAGED;
    }
    else {
        status = STATUS_UNKNOWN;
    }

    Serial.print("engaged: ");
    Serial.println(engaged);
    Serial.print("disengaged: ");
    Serial.println(disengaged);

    Serial.print("Status: ");
    switch (status) {
    case STATUS_ENGAGED:
        Serial.println("engaged");
        break;

    case STATUS_DISENGAGED:
        Serial.println("disengaged");
        break;

    default:
        Serial.println("unknown");
    }

    Serial.println("");

    return status;
}

} // namespace
