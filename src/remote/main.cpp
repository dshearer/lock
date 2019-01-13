/*
Remote main.
*/

#include <Arduino.h>
#include <radio.h>
#include "status_light.h"
#include "control.h"

static uint8_t g_key[16] = {0};

void setup()
{
  Serial.begin(9600);
  radio::setup(g_key, sizeof(g_key), REMOTE_ID);
  status_light::setup();
  control::setup();
}

void loop()
{
  control::loop();
}