/*
Lock main.
*/

#include <Arduino.h>
#include <radio.h>
#include <cryptochip.h>
#include "status_light.h"
#include "status_sensor.h"
#include "motor.h"

static uint8_t g_key[16] = {0};

static void set_status_light()
{
    // TODO
    // const status_t status = g_engaged ? 
    //     STATUS_ENGAGED : STATUS_DISENGAGED;
    // status_light::show_status(status);
}

void engage()
{
    if (status_sensor::getStatus() == STATUS_ENGAGED) {
        return;
    }

    Serial.println(F("Engaging..."));
    motor::runEngage();
    while (status_sensor::getStatus() != STATUS_ENGAGED) {
    }
    motor::stop();

    Serial.println(F("Engaged"));
}

void disengage()
{
    if (status_sensor::getStatus() == STATUS_DISENGAGED) {
        return;
    }

    Serial.println(F("Disengaging..."));
    motor::runDisengage();
    while (status_sensor::getStatus() != STATUS_DISENGAGED) {
        delay(100);
    }
    motor::stop();

    Serial.println(F("Disengaged"));
}

static radio::msg_code_t handle_msg(const radio::msg_t *msg)
{
    radio::msg_code_t resp_code = radio::LOCK_MSG_SUCCESS;

    switch (msg->code) {
    case radio::REMOTE_MSG_ENGAGE:
        Serial.println("Got engage msg");
        engage();
        break;

    case radio::REMOTE_MSG_DISENGAGE:
        Serial.println("Got disengage msg");
        disengage();
        break;

    default:
        Serial.println("Invalid command");
        resp_code = radio::LOCK_MSG_FAILURE;
    }

    set_status_light();
    return resp_code;
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

void loop()
{
    // get msg
    radio::error_t err = radio::ERR_NULL;
    const radio::msg_t *msg = radio::recv(&err);
    if (msg == NULL) {
        if (err != radio::ERR_NULL) {
            Serial.println(F("recv error"));
        }
        return;
    }

    // handle msg/error
    const radio::msg_code_t resp_code = handle_msg(msg);

    // send response
    const radio::msg_t resp_msg = {.code = resp_code};
    if (radio::send(&resp_msg, REMOTE_ID) != 0) {
        Serial.println(F("Failed to send response"));
    }
    radio::setModeRx();
}