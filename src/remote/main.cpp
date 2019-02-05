/*
Remote main.
*/

#include <Arduino.h>
#include <radio.h>
#include <LowPower.h>
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
  // LowPower.idle(SLEEP_FOREVER, ADC_ON, TIMER2_ON, TIMER1_ON, TIMER0_ON, SPI_ON, USART0_ON, TWI_ON);
  Serial.print("Loop\n");
  control::loop();

  Serial.flush();
  LowPower.powerSave(SLEEP_FOREVER, ADC_OFF, BOD_OFF, TIMER2_OFF);
}