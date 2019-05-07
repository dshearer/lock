#ifndef __CRYPTOCHIP_H__
#define __CRYPTOCHIP_H__

#include <words.h>

#define CRYPTOCHIP_DIGEST_LEN 32

namespace cryptochip {

int setup();

const uint8_t *hmac(
    const void *data, size_t data_len,
    const void *key, size_t key_len);

const int counterNext(uint32_t *value);

const uint8_t *random();

}

#endif