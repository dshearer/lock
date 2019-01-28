#include "status_light.h"
#include <Arduino.h>

#define LED_RED_PIN     9
#define LED_GREEN_PIN   6
#define LED_BLUE_PIN    5

#define DISPLAY_TIME_MS 1000

#define LED_HIGH    64

struct color_t
{
    int red;
    int green;
    int blue;

    color_t(int r, int g, int b)
    : red(r), green(g), blue(b) {}
};

static const color_t g_success_color(0, LED_HIGH, 0);
static const color_t g_unauth_color(LED_HIGH, LED_HIGH/3, 0);
static const color_t g_lock_error_color(LED_HIGH, 0, 0);
static const color_t g_radio_error_color(LED_HIGH, 0, 0);

static void showColor(const color_t color)
{
    analogWrite(LED_RED_PIN,    color.red);
    analogWrite(LED_GREEN_PIN,  color.green);
    analogWrite(LED_BLUE_PIN,   color.blue);
    delay(DISPLAY_TIME_MS);
    analogWrite(LED_RED_PIN,    0);
    analogWrite(LED_GREEN_PIN,  0);
    analogWrite(LED_BLUE_PIN,   0);
}

void status_light::setup()
{
    // init pins
    pinMode(LED_RED_PIN,    OUTPUT);
    pinMode(LED_GREEN_PIN,  OUTPUT);
    pinMode(LED_BLUE_PIN,   OUTPUT);

    show_status(STATUS_SUCCESS);
    show_status(STATUS_UNAUTHN);
    show_status(STATUS_LOCK_ERROR);
    show_status(STATUS_RADIO_ERROR);
}

void status_light::show_status(status_t status)
{
    // show light
    switch (status) {
    case STATUS_SUCCESS:
        showColor(g_success_color);
        break;

    case STATUS_UNAUTHN:
        showColor(g_unauth_color);
        break;

    case STATUS_LOCK_ERROR:
        showColor(g_lock_error_color);
        break;

    case STATUS_RADIO_ERROR:
        showColor(g_radio_error_color);
        break;
    }
}