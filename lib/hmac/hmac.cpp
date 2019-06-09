#include <Arduino.h>
#include <string.h>
#include <utils.h>
#include <myassert.h>
#include "hmac.h"

using safearray::ByteArray;

bool Hmac::implOkay() {
    // test
    ByteArray<16> key = {'k', 'e', 'y'};
    ByteArray<5> data = {'H','e','l','l','o'};
    const ByteArray<DIGEST_LEN_BYTES> expected_digest = {
        0xc7, 0x0b, 0x9f, 0x4d, 0x66, 0x5b, 0xd6, 0x29,
        0x74, 0xaf, 0xc8, 0x35, 0x82, 0xde, 0x81, 0x0e,
        0x72, 0xa4, 0x1a, 0x58, 0xdb, 0x82, 0xc5, 0x38,
        0xa9, 0xd7, 0x34, 0xc9, 0x26, 0x6d, 0x32, 0x1e,
    };
    ByteArray<DIGEST_LEN_BYTES> actual_digest = {};
    Hmac hmac;
    hmac.setKey(key.cslice());
    hmac.init() << data >> actual_digest;
    for (size_t i = 0; i < DIGEST_LEN_BYTES; ++i) {
        if (expected_digest[i] != actual_digest[i]) {
            Serial.println(F("Crypto HMAC output is wrong"));
            return -1;
        }
    }
    Serial.println(F("Crypto HMAC output is good"));

    return true;
}

// int compute(CArrayPtr data, CSlice<16> key,
//     Slice<DIGEST_LEN_BYTES> dest)
// {
//     // compute K ^ ipad
//     (g_block << key).fill(0);
//     g_block ^= 0x36;

//     // compute H((K ^ ipad) + data)
//     g_hash.init() << &g_block << data;
//     g_hash.digest(dest);

//     // compute K ^ opad
//     (g_block << key).fill(0);
//     g_block ^= 0x5c;

//     // compute H((K ^ opad) + H((K ^ ipad) + data))
//     g_hash.init() << &g_block << &dest;
//     g_hash.digest(dest.slice());
//     return 0;
// }
