#include <Arduino.h>
#include <radio.h>

static uint8_t g_key[16] = {0};

void setup()
{
  Serial.begin(9600);
  radio::setup(g_key, sizeof(g_key), LOCK_ID);
  radio::setModeRx();
}

void loop()
{
    const radio::msg_t *msg = radio::recv();
    if (msg != NULL) {
        Serial.println("Got msg!");
    }
}