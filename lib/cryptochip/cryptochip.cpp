#include <Arduino.h>
#include <cryptoauthlib.h>
#include "cryptochip.h"

namespace cryptochip {

static bool g_setup_done = false;

int setup()
{
    if (g_setup_done) {
        return 0;
    }

    if (atcab_init(&cfg_ateccx08a_i2c_default) != ATCA_SUCCESS) {
        Serial.println(F("atcab_init failed"));
        return -1;
    }

    // test connection to chip
    if (random() == NULL) {
        return -1;
    }

    g_setup_done = true;
    Serial.println(F("cryptochip setup done"));

    return 0;
}

static uint8_t g_write_buff[32] = {0};
static uint8_t g_digest[CRYPTOCHIP_DIGEST_LEN] = {0};

const uint8_t *hmac(
    const void *data, size_t data_len,
    const void *key, size_t key_len)
{
    ATCA_STATUS res = ATCA_SUCCESS;

    /*
    We load the secret key into slot 00, which has this config:
        SlotConfig: 0x8300
        KeyConfig: 0x3e00
    */

    // load key
    if (key_len > sizeof(g_write_buff)) {
        return NULL;
    }
    memset(g_write_buff, 0, sizeof(g_write_buff));
    memcpy(g_write_buff, key, key_len);
    res = atcab_write_zone(2, 0, 0, 0, g_write_buff, sizeof(g_write_buff));
    if (res != ATCA_SUCCESS) {
        Serial.println(F("Write key failed"));
        Serial.println(res, HEX);
        return NULL;
    }

    // compute HMAC
    res = atcab_sha_hmac((const uint8_t *) data, data_len, 0,
        g_digest, SHA_MODE_TARGET_TEMPKEY);
    if (res != ATCA_SUCCESS) {
        Serial.println(F("HMAC failed"));
        return NULL;
    }
    return g_digest;
}

const int counterNext(uint32_t *value)
{
    if (atcab_counter_increment(0, value) != ATCA_SUCCESS) {
        Serial.println(F("atcab_counter_increment failed"));
        return -1;
    }
    return 0;
}

static uint8_t g_buff[32] = {0};

const uint8_t *random()
{
    if (atcab_random(g_buff) != ATCA_SUCCESS) {
        Serial.println(F("atcab_random failed"));
        return NULL;
    }
    return g_buff;
}

}