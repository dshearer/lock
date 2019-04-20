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
static volatile status_t g_status = STATUS_UNKNOWN;
static callback_t g_callbacks[8] = {0};
static size_t g_nbrCallbacks = 0;

/**
 * Updates g_status based on values of g_engaged and g_disengaged.
 * Returns previous status.
 */
static status_t updateStatus()
{
    /*
    There is a (very unlikely) race-condition here, where both
    buttons seem to be depressed.  In this case, we just
    read them again after a short wait.
    */

    const bool engaged = digitalRead(ENGAGED_SENSOR_PIN) == BTN_PRESSED_LEVEL;
    const bool disengaged = digitalRead(DISENGAGED_SENSOR_PIN) == BTN_PRESSED_LEVEL;

    const status_t oldStatus = g_status;
    bool tryingAgain = false;

tryAgain:
    if (engaged && disengaged) {
        if (!tryingAgain) {
            delay(10);
            tryingAgain = true;
            goto tryAgain;
        }
        else {
            g_status = STATUS_UNKNOWN;
        }
    }
    else if (engaged) {
        g_status = STATUS_ENGAGED;
    }
    else if (disengaged) {
        g_status = STATUS_DISENGAGED;
    }
    else {
        g_status = STATUS_UNKNOWN;
    }

    Serial.print("engaged: ");
    Serial.println(engaged);
    Serial.print("disengaged: ");
    Serial.println(disengaged);

    Serial.print("Status: ");
    Serial.println(g_status);

    Serial.println("");

    return oldStatus;
}

static void handleStatusChange()
{
    const status_t oldStatus = updateStatus();
    for (size_t i = 0; i < g_nbrCallbacks; ++i) {
        g_callbacks[i](oldStatus, g_status);
    }
}

void setup()
{
    // enable internal pullup resistors for button pins
    pinMode(ENGAGED_SENSOR_PIN,     INPUT_PULLUP);
    pinMode(DISENGAGED_SENSOR_PIN,  INPUT_PULLUP);

    // get status
    updateStatus();

    // attach interrupts
    attachInterrupt(digitalPinToInterrupt(ENGAGED_SENSOR_PIN),
        handleStatusChange, CHANGE);
    attachInterrupt(digitalPinToInterrupt(DISENGAGED_SENSOR_PIN), 
        handleStatusChange, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(ENGAGED_SENSOR_PIN),
    //     handleEngagedReleased, BTN_RELEASED_INTERRUPT_LEVEL);
    // attachInterrupt(digitalPinToInterrupt(DISENGAGED_SENSOR_PIN), 
    //     handleDisengagedReleased, BTN_RELEASED_INTERRUPT_LEVEL);
}

void addCallback(callback_t cb)
{
    if (g_nbrCallbacks >= NUM_ELEMS(g_callbacks)) {
        return;
    }

    g_callbacks[g_nbrCallbacks] = cb;
    ++g_nbrCallbacks;
}

status_t getStatus()
{
    return g_status;
}

} // namespace
