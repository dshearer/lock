#ifndef __HMAC_H__
#define __HMAC_H__

#include <inttypes.h>
#include <sha3.h>

#define HASH_INST           sha3::SHA3_INST_224
#define DIGEST_LEN_BYTES    (HASH_INST / 8)

namespace hmac {

/*
@return A buffer of length DIGEST_LEN_BYTES.
*/
const uint8_t *compute(
    const void *data, size_t data_len,
    const void *key, size_t key_len);

bool verify(
    const void *data, size_t data_len,
    const void *key, size_t key_len,
    const uint8_t *given_digest_1, const uint8_t *given_digest_2);

}

#endif /* __HMAC_H__ */