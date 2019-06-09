/*
Lock main.
*/

#include <Arduino.h>
#include <radio.h>
#include "status_light.h"
#include "status_sensor.h"
#include "motor.h"

#define ACTUATE_TIMEOUT_MILLIS 5000

using safearray::ByteArray;

static ByteArray<KEY_LEN_BYTES> g_key = {};

static void set_status_light()
{
    // TODO
    // const status_t status = g_engaged ? 
    //     STATUS_ENGAGED : STATUS_DISENGAGED;
    // status_light::show_status(status);
}

static int actuate(void (*actuate_f)(void), status_t targetStatus) {
    if (status_sensor::getStatus() == targetStatus) {
        return 0;
    }

    const unsigned long stopTime = millis() + ACTUATE_TIMEOUT_MILLIS;
    actuate_f();
    while (status_sensor::getStatus() != targetStatus && millis() < stopTime) {
    }
    motor::stop();

    return status_sensor::getStatus() == targetStatus ? 0 : -1;
}

static int engage()
{
    Serial.println(F("Engaging..."));
    return actuate(motor::runEngage, STATUS_ENGAGED);
}

static int disengage()
{
    Serial.println(F("Disengaging..."));
    return actuate(motor::runDisengage, STATUS_DISENGAGED);
}

static radio::msg_code_t handle_msg(const radio::msg_t *msg)
{
    radio::msg_code_t resp_code = radio::LOCK_MSG_SUCCESS;

    switch (msg->code) {
    case radio::REMOTE_MSG_ENGAGE:
        Serial.println(F("Got engage msg"));
        if (engage() != 0) {
            Serial.println(F("Engage failed"));
            resp_code = radio::LOCK_MSG_FAILURE;
        }
        break;

    case radio::REMOTE_MSG_DISENGAGE:
        Serial.println(F("Got disengage msg"));
        if (disengage() != 0) {
            Serial.println(F("Disengage failed"));
            resp_code = radio::LOCK_MSG_FAILURE;
        }
        break;

    default:
        Serial.println(F("Invalid command"));
        resp_code = radio::LOCK_MSG_FAILURE;
    }

    set_status_light();
    return resp_code;
}

void setup()
{
  Serial.begin(9600);
  radio::setup(g_key.cslice(), LOCK_ID);
  status_light::setup();
  status_sensor::setup();
  motor::setup();
  set_status_light();
}

void loop() {
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
}

#if 0
void loop() {
    static Array<11> data;
    static Array<DIGEST_LEN_BYTES> digest;

    Serial.println("Computing hash");
    int res = hmac::compute(CArrayPtr(data), g_key.cslice(), digest.slice());
    if (res != 0) {
        Serial.println("hmac::compute failed");
        return;
    }

    Serial.println("Verifying hash");
    bool vres = hmac::verify(CArrayPtr(data), g_key.cslice(), digest.cslice());
    if (vres) {
        Serial.println("Verified");
    }
    else {
        Serial.println("Not verified");
    }
}
#endif

#if 0
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
#endif