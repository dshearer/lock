#include "status_light.h"
#include <Arduino.h>

#define LED_RED_PIN     9
#define LED_GREEN_PIN   6
#define LED_BLUE_PIN    5

#define LED_HIGH    64
 
struct color_t
{
    int red;
    int green;
    int blue;

    color_t(int r, int g, int b)
    : red(r), green(g), blue(b) {}
};

static const color_t g_engaged_color(LED_HIGH, 0, 0);
static const color_t g_disengaged_color(0, LED_HIGH, 0);

static void showColor(const color_t color)
{
    analogWrite(LED_RED_PIN,    color.red);
    analogWrite(LED_GREEN_PIN,  color.green);
    analogWrite(LED_BLUE_PIN,   color.blue);
}

void status_light::setup()
{
    // init pins
    pinMode(LED_RED_PIN,    OUTPUT);
    pinMode(LED_GREEN_PIN,  OUTPUT);
    pinMode(LED_BLUE_PIN,   OUTPUT);
}

void status_light::show_status(status_t status)
{
    Serial.println("Setting status");
    Serial.println(status);

    // show light
    switch (status) {
    case STATUS_ENGAGED:
        showColor(g_engaged_color);
        break;

    default:
        showColor(g_disengaged_color);
        break;
    }
}