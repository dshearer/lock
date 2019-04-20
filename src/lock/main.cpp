/*
Lock main.
*/

#include <Arduino.h>
#include <radio.h>
#include "status_light.h"
#include "status_sensor.h"
#include "motor.h"

#define TESTING

static uint8_t g_key[16] = {0};
static bool g_engaged = false;

static void set_status_light()
{
    const status_t status = g_engaged ? 
        STATUS_ENGAGED : STATUS_DISENGAGED;
    status_light::show_status(status);
}

// static void engageCallback(status_t oldStatus, status_t newStatus)
// {
//     if (newStatus != STATUS_ENGAGED) {
//         return;
//     }
//     motor::stop();
// }

static void engage()
{
    if (status_sensor::getStatus() == STATUS_ENGAGED) {
        return;
    }

    motor::runLeft();
    while (status_sensor::getStatus() != STATUS_ENGAGED) {
        delay(100);
    }
    motor::stop();

    Serial.println("Engaged");
}

// static void disengageCallback(status_t oldStatus, status_t newStatus)
// {
//     if (newStatus != STATUS_DISENGAGED) {
//         return;
//     }

// }

static void disengage()
{
    if (status_sensor::getStatus() == STATUS_DISENGAGED) {
        return;
    }

    motor::runRight();
    while (status_sensor::getStatus() != STATUS_DISENGAGED) {
        delay(100);
    }
    motor::stop();

    Serial.println("Disengaged");
}

void setup()
{
  Serial.begin(9600);
  radio::setup(g_key, sizeof(g_key), LOCK_ID);
  radio::setModeRx();
  status_light::setup();
  status_sensor::setup();
  motor::setup();
  set_status_light();
}

#ifndef TESTING
static radio::msg_code_t handle_msg(const radio::msg_t *msg)
{
    radio::msg_code_t resp_code = radio::LOCK_MSG_SUCCESS;

    Serial.println("Got msg!");

    switch (msg->code) {
    case radio::REMOTE_MSG_ENGAGE:
        if (g_engaged) {
            goto done;
        }

        // engage lock...
        Serial.println("Engaging lock");
        g_engaged = true;
        break;

    case radio::REMOTE_MSG_DISENGAGE:
        if (!g_engaged) {
            goto done;
        }

        // disengage lock...
        Serial.println("Disengaging lock");
        g_engaged = false;
        break;

    default:
        resp_code = radio::LOCK_MSG_FAILURE;
    }

done:
    set_status_light();
    return resp_code;
}

void loop()
{
    // get msg
    radio::error_t err = radio::ERR_NULL;
    const radio::msg_t *msg = radio::recv(&err);
    if (msg == NULL && err == radio::ERR_NULL) {
        return;
    }

    // handle msg/error
    Serial.println(F("Got msg"));
    radio::msg_code_t resp_code = radio::LOCK_MSG_FAILURE;
    if (err != radio::ERR_NULL) {
        resp_code = radio::LOCK_MSG_UNAUTHN;
    }
    else {
        resp_code = handle_msg(msg);
    }

    // send response
    radio::msg_t resp_msg = {.code = resp_code};
    if (radio::send(&resp_msg, REMOTE_ID) != 0) {
        Serial.println(F("Failed to send response"));
    }
    radio::setModeRx();
}

#else

void loop()
{
    bool flag = false;
    while (true) {
        if (flag) {
            engage();
            flag = false;
        }
        else {
            disengage();
            flag = true;
        }

        delay(3000);
    }
}
#endif