#include <Arduino.h>
#include "print.h"

extern "C" {

void println(const char *s)
{
    Serial.println(s);
}

}