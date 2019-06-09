#ifdef HMAC_SW_IMPL

#include <Arduino.h>
#include <fail.h>
#include "hmac.h"

using safearray::ByteSlice;
using safearray::CByteSlice;

static_assert(DIGEST_LEN_BYTES == ATCA_SHA2_256_DIGEST_SIZE,
    "Bad digest length");

Sha256& Sha256::init() {
    atcac_sw_sha2_256_init(&this->_ctx);
    this->_inited = true;
    return *this;
}

void Sha256::update(const void *data, size_t size) {
    if (!this->_inited) {
        fail(F("Sha256 not inited"));
    }
    atcac_sw_sha2_256_update(&this->_ctx, (const uint8_t *) data, size);
}

void Sha256::operator>>(ByteSlice<DIGEST_LEN_BYTES> dest) {
    if (!this->_inited) {
        fail(F("Sha256 not inited"));
    }
    atcac_sw_sha2_256_finish(&this->_ctx, (uint8_t *) dest.data());
}

Hmac::Hmac() {}

void Hmac::setKey(CByteSlice<HMAC_KEY_LEN_BYTES> key) {
    this->_key = key;
}

Hmac& Hmac::init() {
    if (this->_key.cdata() == NULL) {
        fail(F("Key not set"));
    }

    // compute K ^ ipad
    (this->_block << this->_key).fill(0);
    this->_block ^= 0x36;

    // start computing H((K ^ ipad) + data)
    this->_sha.init() << this->_block;

    this->_inited = true;
    return *this;
}

void Hmac::update(const void *data, size_t size) {
    if (!this->_inited) {
        fail(F("Hmac not inited"));
    }

    // continue computing H(this->_block + data)
    this->_sha.update(data, size);
}

void Hmac::operator>>(ByteSlice<DIGEST_LEN_BYTES> dest) {
    if (!this->_inited) {
        fail(F("Hmac not inited"));
    }

    // finish computing H(this->_block + data)
    this->_sha >> dest;

    // compute K ^ opad
    (this->_block << this->_key).fill(0);
    this->_block ^= 0x5c;

    // compute H((K ^ opad) + H((K ^ ipad) + data))
    this->_sha.init() << this->_block << dest >> dest;
}

#endif // HMAC_SW_IMPL