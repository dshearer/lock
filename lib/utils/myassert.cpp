#include <Arduino.h>
#include "myassert.h"

void __assert(const char *func, const char *file, int lineno, const char *sexp) {
    Serial.print(F("ASSERT FAILURE: "));
    Serial.print(file);
    Serial.print(F(":"));
    Serial.print(lineno, DEC);
    Serial.print(F(": "));
    Serial.print(func);
    Serial.print(F(": "));
    Serial.println(sexp);
    Serial.flush();
    abort();
}