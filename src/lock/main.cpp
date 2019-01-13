/*
Lock main.
*/

#include <Arduino.h>
#include <radio.h>
#include "status_light.h"

static uint8_t g_key[16] = {0};
static bool g_engaged = false;

static void set_status_light()
{
    const status_light::status_t status = g_engaged ? 
        status_light::STATUS_ENGAGED : 
        status_light::STATUS_DISENGAGED;
    status_light::show_status(status);
}

void setup()
{
  Serial.begin(9600);
  radio::setup(g_key, sizeof(g_key), LOCK_ID);
  radio::setModeRx();
  status_light::setup();
  set_status_light();
}

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