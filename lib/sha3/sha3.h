#ifndef __SHA3_H__
#define __SHA3_H__

#include <stddef.h>

namespace sha3 {
    typedef enum {
        SHA3_INST_224 = 224,
        SHA3_INST_256 = 256,
        SHA3_INST_384 = 384,
        SHA3_INST_512 = 512,
    } sha3_inst_t;

    void init(sha3_inst_t inst);
    void update(const void *input, size_t len);
    const void *finalize(unsigned *len);
}

#ifdef LOCAL_TEST
#include <stdio.h>
static void print_hex(const void *hash, size_t len)
{
    uint8_t *p = (uint8_t *) hash;

    for (size_t i = 0; i < len; ++i)
    {
        if (i % 16 == 0) {
            printf("\n");
        }

        printf("%02hhx ", p[i]);
    }
    printf("\n");
}
#endif

#endif /* __SHA3_H__ */