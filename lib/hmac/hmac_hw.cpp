#ifndef HMAC_SW_IMPL

#include <Arduino.h>
#include <fail.h>
#include "hmac.h"

#define KEY_ZONE 2
#define KEY_SLOT 0

using safearray::CByteSlice;
using safearray::ByteSlice;

static_assert(DIGEST_LEN_BYTES == ATCA_SHA2_256_DIGEST_SIZE,
    "Bad digest length");

static bool g_cryptochip_inited = false;

static void init_cryptochip() {
    if (g_cryptochip_inited) {
        return;
    }

    if (atcab_init(&cfg_ateccx08a_i2c_default) != ATCA_SUCCESS) {
        fail(F("atcab_init failed"));
    }
    g_cryptochip_inited = true;
}

Hmac::Hmac() {}

void Hmac::setKey(CByteSlice<HMAC_KEY_LEN_BYTES> key) {
    init_cryptochip();

    /*
    We load the secret key into slot 00, which has this config:
        SlotConfig: 0x8300
        KeyConfig: 0x3e00
    */

    // load key onto chip
    this->_write_buff.fill(0);
    this->_write_buff.assign(key);
    ATCA_STATUS res = atcab_write_zone(
        KEY_ZONE, // zone
        KEY_SLOT, // slot
        0, // block
        0, // offset
        (const uint8_t *) this->_write_buff.data(), // data
        this->_write_buff.size()); // len
    if (res != ATCA_SUCCESS) {
        fail(F("atcab_write_zone failed"));
    }

    this->_keyLoaded = true;
}

Hmac& Hmac::init() {
    if (!this->_keyLoaded) {
        fail(F("Key not set"));
    }

    // begin HMAC operation
    ATCA_STATUS res = atcab_sha_hmac_init(&this->_ctx, KEY_SLOT);
    if (res != ATCA_SUCCESS) {
        fail(F("atcab_sha_hmac_init failed"));
    }

    this->_inited = true;
    return *this;
}

void Hmac::update(const void *data, size_t size) {
    if (!this->_inited) {
        fail(F("Hmac not inited"));
    }

    ATCA_STATUS res = atcab_sha_hmac_update(&this->_ctx,
        (const uint8_t *) data, size);
    if (res != ATCA_SUCCESS) {
        fail(F("atcab_sha_hmac_update failed"));
    }
}

void Hmac::operator>>(ByteSlice<DIGEST_LEN_BYTES> dest) {
    if (!this->_inited) {
        fail(F("Hmac not inited"));
    }

    ATCA_STATUS res = atcab_sha_hmac_finish(&this->_ctx, dest.data(),
        SHA_MODE_TARGET_TEMPKEY);
    if (res != ATCA_SUCCESS) {
        fail(F("atcab_sha_hmac_finish failed"));
    }
}

#endif // !HMAC_SW_IMPL