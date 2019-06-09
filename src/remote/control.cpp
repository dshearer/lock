#include <radio.h>
#include <utils.h>
#include "control.h"
#include "status_light.h"

#define ENGAGE_BTN_PIN      2
#define DISENGAGE_BTN_PIN   3

#define FEEDBACK_LED_PIN    4

#define BTN_PRESSED_LEVEL   LOW

#define REPLY_TIMEOUT_SECS  7

namespace control {

/*
We handle button presses with interrupts, but the actual logic is done in
the loop.
*/

static volatile uint8_t g_pushed_button = 0;

static void handleEngagePressed()
{
    g_pushed_button = ENGAGE_BTN_PIN;
}

static void handleDisengagePressed()
{
    g_pushed_button = DISENGAGE_BTN_PIN;
}

static status_t getResponse()
{
    // get response
    radio::error_t err = radio::ERR_NULL;
    const radio::msg_t *resp = radio::recvTimeout(REPLY_TIMEOUT_SECS, &err);

    // handle error
    if (err != radio::ERR_NULL) {
        return STATUS_RADIO_ERROR;
    }

    // handle response
    ASSERT(resp != NULL);
    switch (resp->code)
    {
    case radio::LOCK_MSG_SUCCESS:
        return STATUS_SUCCESS;

    case radio::LOCK_MSG_UNAUTHN:
        return STATUS_UNAUTHN;

    default:
        return STATUS_LOCK_ERROR;
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
        Serial.println(F("Failed to send cmd"));
        return;
    }

    // get response
    const status_t status = getResponse();

    // show status
    status_light::showStatus(status);
    Serial.print(F("Lock resp: "));
    status_light::printStatus(status);
    Serial.println(F(""));

    const unsigned long time_diff = millis() - start_time;
    Serial.print(F("Op time: "));
    Serial.println(time_diff);
}

static void attachInterrupts()
{
    attachInterrupt(digitalPinToInterrupt(ENGAGE_BTN_PIN),
        handleEngagePressed, BTN_PRESSED_LEVEL);
    attachInterrupt(digitalPinToInterrupt(DISENGAGE_BTN_PIN), 
        handleDisengagePressed, BTN_PRESSED_LEVEL);
}

static void detachInterrupts()
{
    detachInterrupt(digitalPinToInterrupt(ENGAGE_BTN_PIN));
    detachInterrupt(digitalPinToInterrupt(DISENGAGE_BTN_PIN));
}

void setup()
{
    // enable internal pullup resistors for button pins
    pinMode(ENGAGE_BTN_PIN,     INPUT_PULLUP);
    pinMode(DISENGAGE_BTN_PIN,  INPUT_PULLUP);

    // init other pins
    pinMode(FEEDBACK_LED_PIN, OUTPUT);

    // set interrupt handlers
    attachInterrupts();
}

void loop()
{
    if (g_pushed_button != 0) {
        detachInterrupts();
    }

    switch (g_pushed_button) {
    case ENGAGE_BTN_PIN:
        Serial.println(F("Engage pushed"));
        digitalWrite(FEEDBACK_LED_PIN, HIGH);
        sendCmd(radio::REMOTE_MSG_ENGAGE);
        digitalWrite(FEEDBACK_LED_PIN, LOW);
        break;

    case DISENGAGE_BTN_PIN:
        Serial.println(F("Disengage pushed"));
        digitalWrite(FEEDBACK_LED_PIN, HIGH);
        sendCmd(radio::REMOTE_MSG_DISENGAGE);
        digitalWrite(FEEDBACK_LED_PIN, LOW);
        break;
    }

    if (g_pushed_button != 0) {
        attachInterrupts();
    }

    g_pushed_button = 0;
}

}