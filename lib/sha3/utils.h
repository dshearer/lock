#ifndef __UTILS_H__
#define __UTILS_H__

#ifndef MIN
#  define MIN(_x, _y) ((_x) < (_y) ? (_x) : (_y))
#endif

#ifdef NDEBUG
#  define ASSERT
#elif defined(LOCAL_TEST)
#  include <assert.h>
#  define ASSERT assert
#else
#  include <Arduino.h>
#  define ASSERT(_exp) do { \
    if (_exp) { \
        break; \
    } \
    \
    Serial.println("Check failed: " #_exp); \
    Serial.flush(); \
    while (true) {}; \
} while (false)
#endif

#endif /* __UTILS_H__ */