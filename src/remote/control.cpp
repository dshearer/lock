#include <radio.h>
#include <utils.h>
#include "control.h"
#include "status_light.h"

#define ENGAGE_BTN_PIN      5
#define DISENGAGE_BTN_PIN   2

#define BTN_PRESSED_LEVEL   LOW

#define REPLY_TIMEOUT_SECS  5

namespace control {

void setup()
{
    // enable internal pullup resistors for button pins
    pinMode(ENGAGE_BTN_PIN,     INPUT_PULLUP);
    pinMode(DISENGAGE_BTN_PIN,  INPUT_PULLUP);
}

static status_light::status_t getResponse()
{
    // get response
    radio::error_t err = radio::ERR_NULL;
    const radio::msg_t *resp = radio::recvTimeout(REPLY_TIMEOUT_SECS, &err);

    // handle error
    if (err != radio::ERR_NULL) {
        return status_light::STATUS_RADIO_ERROR;
    }

    // handle response
    ASSERT(resp != NULL);
    switch (resp->code)
    {
    case radio::LOCK_MSG_SUCCESS:
        return status_light::STATUS_SUCCESS;

    case radio::LOCK_MSG_UNAUTHN:
        return status_light::STATUS_UNAUTHN;

    default:
        return status_light::STATUS_LOCK_ERROR;
    }
}

static void sendCmd(radio::msg_code_t cmd)
{
    /*
    1. Send command to lock.
    2. Get reply, which says whether operation was successful.
    3. Show status in LEDs.
    */

   const unsigned long start_time = millis();

    // send command
    const radio::msg_t msg = radio::msg_t{.code = cmd};
    if (radio::send(&msg, LOCK_ID) != 0) {
        Serial.println("Failed to send cmd");
        return;
    }

    // get response
    const status_light::status_t status = getResponse();

    // show status
    status_light::show_status(status);

    const unsigned long time_diff = millis() - start_time;
    Serial.print("Op time: ");
    Serial.println(time_diff);
}

void loop()
{
    if (digitalRead(ENGAGE_BTN_PIN) == BTN_PRESSED_LEVEL) {
        sendCmd(radio::REMOTE_MSG_ENGAGE);
    }
    else if (digitalRead(DISENGAGE_BTN_PIN) == BTN_PRESSED_LEVEL) {
        sendCmd(radio::REMOTE_MSG_DISENGAGE);
    }
}

}