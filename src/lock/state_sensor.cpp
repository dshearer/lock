#include <Arduino.h>
#include "state_sensor.h"

#define ENGAGED_SENSOR_PIN      4
#define DISENGAGED_SENSOR_PIN   5

#define BTN_PRESSED_LEVEL   LOW

namespace status_sensor {

/*
There are two buttons, one of which is depressed when the lock is engaged
and one of which is depressed when the lock is disengaged.

It is not possible for both to be depressed, but it is possible for
neither of them to be.  In this case, the lock is neither engaged nor
disengaged, and its effectiveness at keeping the door closed is unknown.
*/

static status_t g_status = STATUS_UNKNOWN;

void setup()
{
    // enable internal pullup resistors for button pins
    pinMode(ENGAGED_SENSOR_PIN,     INPUT_PULLUP);
    pinMode(DISENGAGED_SENSOR_PIN,  INPUT_PULLUP);
}

status_t _getStatus()
{
    return _getStatus(false);
}

static status_t _getStatus(bool tryingAgain)
{
    /*
    There is a (very unlikely) race-condition here, where both
    buttons seem to be depressed.  In this case, we just
    read them again after a short wait.
    */

    const bool engaged = digitalRead(ENGAGED_SENSOR_PIN) == BTN_PRESSED_LEVEL;
    const bool disengaged = digitalRead(DISENGAGED_SENSOR_PIN) 
        == BTN_PRESSED_LEVEL;

    if (engaged && disengaged) {
        if (!tryingAgain) {
            delay(10);
            return _getStatus(true);
        }
        else {
            return STATUS_UNKNOWN;
        }
    }
    else if (engaged) {
        return STATUS_ENGAGED;
    }
    else if (disengaged) {
        return STATUS_DISENGAGED;
    }
    else {
        return STATUS_UNKNOWN;
    }
}

}