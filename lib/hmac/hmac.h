#ifndef __HMAC_H__
#define __HMAC_H__

#include <array.h>
#include <cryptoauthlib.h>

#define DIGEST_LEN_BYTES     32
#define HMAC_KEY_LEN_BYTES   16
#define HMAC_BLOCK_LEN_BYTES 64 // https://tools.ietf.org/html/rfc4868#section-2.2

#ifdef HMAC_SW_IMPL
class Sha256 {
public:
    Sha256& init();
    void update(const void *data, size_t size);
    void operator>>(Slice<DIGEST_LEN_BYTES> dest);

    inline Sha256& operator<<(CArrayPtr data) {
        this->update(data.data(), data.size());
        return *this;
    }

    template<int L>
    inline Sha256& operator<<(Array<L>& data) {
        this->update(data.data(), data.size());
        return *this;
    }

    template<int L>
    inline Sha256& operator<<(Slice<L>& data) {
        this->update(data.data(), data.size());
        return *this;
    }

private:
    bool _inited = false;
    atcac_sha2_256_ctx _ctx;
};
#endif // HMAC_SW_IMPL

class Hmac {
public:
    Hmac();

    void setKey(CSlice<HMAC_KEY_LEN_BYTES> key);
    Hmac& init();
    void update(const void *data, size_t size);
    void operator>>(Slice<DIGEST_LEN_BYTES> dest);

    template<int L>
    inline void operator>>(Array<L>& dest) {
        *this >> dest.template slice<0, DIGEST_LEN_BYTES>();
    }

    template<int L>
    inline Hmac& operator<<(const Array<L>& data) {
        this->update(data.data(), L);
        return *this;
    }

    template<typename T>
    inline Hmac& operator<<(const T *obj) {
        this->update(obj, sizeof(*obj));
        return *this;
    }

    static bool implOkay();

    template<int L>
    static bool digestsEqual(CSlice<L> d1, CSlice<L> d2) {
        bool mismatch = false;
        for (size_t i = 0; i < d1.size(); ++i) { // avoid timing-attacks
            if (d1[i] != d2[i]) {
                mismatch = true;
            }
        }
        return !mismatch;
    }
    
private:
    bool _inited = false;
#ifdef HMAC_SW_IMPL
    Array<HMAC_BLOCK_LEN_BYTES> _block = {};
    CSlice<HMAC_KEY_LEN_BYTES> _key;
    Sha256 _sha;
#else
    atca_hmac_sha256_ctx_t _ctx;
    bool _keyLoaded = false;
    Array<32> _write_buff = {};
#endif
};

#endif /* __HMAC_H__ */