#include <Arduino.h>
#include <radio.h>

static uint8_t g_key[16] = {0};

void setup()
{
  Serial.begin(9600);
  radio::setup(g_key, sizeof(g_key), REMOTE_ID);
  radio::setModeIdle();
}

void loop()
{
  Serial.println("Sending 'lock' msg");
  radio::msg_t msg = {.code = radio::REMOTE_MSG_ENGAGE};
  if (radio::send(&msg, LOCK_ID) != 0) {
      /* Error */
      Serial.println("radio::send error");
  }
  
  radio::setModeIdle();
  delay(500);
  Serial.println("");
}