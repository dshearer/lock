#ifndef __HMAC_H__
#define __HMAC_H__

#include <mcu_safe_array.h>
#include <cryptoauthlib.h>

#define DIGEST_LEN_BYTES     32
#define HMAC_KEY_LEN_BYTES   16
#define HMAC_BLOCK_LEN_BYTES 64 // https://tools.ietf.org/html/rfc4868#section-2.2

#ifdef HMAC_SW_IMPL
class Sha256 {
public:
    Sha256& init();
    void update(const void *data, size_t size);
    void operator>>(safearray::ByteSlice<DIGEST_LEN_BYTES> dest);

    inline Sha256& operator<<(safearray::CByteArrayPtr data) {
        this->update(data.data(), data.size());
        return *this;
    }

    template<size_t L>
    inline Sha256& operator<<(safearray::ByteArray<L>& data) {
        this->update(data.data(), data.size());
        return *this;
    }

    template<size_t L>
    inline Sha256& operator<<(safearray::ByteSlice<L>& data) {
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

    void setKey(safearray::CByteSlice<HMAC_KEY_LEN_BYTES> key);
    Hmac& init();
    void update(const void *data, size_t size);
    void operator>>(safearray::ByteSlice<DIGEST_LEN_BYTES> dest);

    template<size_t L>
    inline void operator>>(safearray::ByteArray<L>& dest) {
        *this >> dest.template slice<0, DIGEST_LEN_BYTES>();
    }

    template<size_t L>
    inline Hmac& operator<<(const safearray::ByteArray<L>& data) {
        this->update(data.cdata(), L);
        return *this;
    }

    template<typename T>
    inline Hmac& operator<<(const T *obj) {
        this->update(obj, sizeof(*obj));
        return *this;
    }

    static bool implOkay();

    template<size_t L>
    static bool digestsEqual(safearray::CByteSlice<L> d1, safearray::CByteSlice<L> d2) {
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
    safearray::ByteArray<HMAC_BLOCK_LEN_BYTES> _block = {};
    safearray::CByteSlice<HMAC_KEY_LEN_BYTES> _key;
    Sha256 _sha;
#else
    atca_hmac_sha256_ctx_t _ctx;
    bool _keyLoaded = false;
    safearray::ByteArray<32> _write_buff = {};
#endif
};

#endif /* __HMAC_H__ */