#ifndef __FAIL_H__
#define __FAIL_H__

#define fail(_msg) \
do { \
    Serial.print(F("FAIL: ")); \
    Serial.println(_msg); \
    Serial.flush(); \
    abort(); \
} while (false)

#endif // __FAIL_H__