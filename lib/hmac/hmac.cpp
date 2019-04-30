#include <Arduino.h>
#include <string.h>
#include <cryptoauthlib.h>
#include <utils.h>
#include "hmac.h"

#define HMAC_BLOCK_LEN_BYTES 144 // http://www.wolfgang-ehrhardt.de/hmac-sha3-testvectors.html

namespace hmac {

static uint8_t g_digest[DIGEST_LEN_BYTES] = {0};

#ifdef HMAC_OWN_IMPL

static uint8_t g_block[HMAC_BLOCK_LEN_BYTES] = {0};

static void pad_key(void *dest, size_t dest_len, const void *key, size_t key_len)
{
    const size_t copy_len = MIN(key_len, dest_len);
    memcpy(dest, key, copy_len);
    memset(((uint8_t *) dest) + copy_len, 0, dest_len - copy_len);
}

static void xor_with_const(uint8_t *dest, size_t dest_len, uint8_t value)
{
    for (size_t i = 0; i < dest_len; ++i)
    {
        dest[i] ^= value;
    }
}

int init()
{
    return 0;
}

const uint8_t *compute(
    const void *data, size_t data_len,
    const void *key, size_t key_len)
{
    // compute K ^ ipad
    pad_key(g_block, sizeof(g_block), key, key_len);
    xor_with_const(g_block, sizeof(g_block), 0x36);

    // compute H(K ^ ipad + data)
    sha3::init(HASH_INST);
    // err start
    sha3::update(g_block, sizeof(g_block));
    // err end
    sha3::update(data,    data_len);
    const void *tmp = sha3::finalize(NULL);
    memcpy(g_digest, tmp, sizeof(g_digest));

    // compute K ^ opad
    pad_key(g_block, sizeof(g_block), key, key_len);
    xor_with_const(g_block, sizeof(g_block), 0x5c);

    // compute H(K ^ opad + H(K ^ ipad + data))
    sha3::init(HASH_INST);
    sha3::update(g_block,  sizeof(g_block));
    sha3::update(g_digest, sizeof(g_digest));
    return (const uint8_t *) sha3::finalize(NULL);
}

#else // HMAC_OWN_IMPL

int init()
{
    if (atcab_init(&cfg_ateccx08a_i2c_default) != ATCA_SUCCESS) {
        return -1;
    }

    // test connection to chip
    if (atcab_random(NULL) != ATCA_SUCCESS) {
        Serial.println("Rand failed!");
        return -1;
    }
    Serial.println("Rand succeeded");

    return 0;
}

static uint8_t g_write_buff[32] = {0};

const uint8_t *compute(
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

#endif // HMAC_OWN_IMPL

bool verify(
    const void *data, size_t data_len,
    const void *key, size_t key_len,
    const uint8_t *given_digest_1, const uint8_t *given_digest_2)
{
    const uint8_t *exp_digest = compute(data, data_len, key, key_len);
    if (memcmp(exp_digest, given_digest_1, DIGEST_LEN_BYTES/2) != 0) {
        return false;
    }
    if (memcmp(exp_digest + DIGEST_LEN_BYTES/2, given_digest_2, 
        DIGEST_LEN_BYTES/2) != 0) {
        return false;
    }
    return true;
}

} // namespace