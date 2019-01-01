#ifndef __WORDS_H__
#define __WORDS_H__

#include <inttypes.h>

template<unsigned B>
class Word
{
    /* big-endian */

public:
    Word()
    {
        this->zero_out();
    }

    void zero_out()
    {
        *this ^= *this;
    }

    const uint8_t& operator[](unsigned idx) const
    {
        return this->_b[idx];
    }

    uint8_t& operator[](unsigned idx)
    {
        return this->_b[idx];
    }

    Word& operator^=(const Word& other)
    {
        for (unsigned i = 0; i < B; ++i)
        {
            (*this)[i] ^= other[i];
        }
        return *this;
    }

    Word operator^(const Word& other) const
    {
        Word w2 = *this;
        w2 ^= other;
        return w2;
    }

    Word operator&(const Word& other) const
    {
        Word w2;
        for (unsigned i = 0; i < B; ++i)
        {
            w2[i] = (*this)[i] & other[i];
        }
        return w2;
    }

    Word operator~() const
    {
        Word w2;
        for (unsigned i = 0; i < B; ++i)
        {
            w2[i] = ~(*this)[i];
        }
        return w2;
    }

    Word& operator++()
    {
        this->inc(B - 1);
        return *this;
    }

    bool operator<(const Word& other) const
    {
        for (unsigned i = 0; i < B; ++i)
        {
            const uint8_t v1 = (*this)[i];
            const uint8_t v2 = other[i];
            if (v1 < v2) {
                return true;
            }
            else if (v1 > v2) {
                return false;
            }
        }
        return false;
    }

    void rotl(unsigned n)
    {
        this->flip();
        for (unsigned i = 0; i < n; ++i)
        {
            this->rotl_1();
        }
        this->flip();
    }

    void flip()
    {
        Word tmp;
        for (unsigned i = 0; i < B; ++i)
        {
            tmp[i] = (*this)[B - i - 1];
        }
        *this = tmp;
    }

private:
    void rotl_1()
    {
        const bool last = (*this)[0] >> 7;
        for (unsigned i = 0; i < B; ++i)
        {
            // shift byte
            (*this)[i] <<= 1;

            // set byte's last bit
            bool curr_last = last;
            if (i < B - 1) {
                curr_last = (*this)[i + 1] >> 7;
            }
            if (curr_last) {
                (*this)[i] |= 1;
            }
            else {
                (*this)[i] &= ~1;
            }
        }
    }

    void inc(int pos)
    {
        if (pos < 0) {
            return;
        }

        ++(*this)[pos];

        if ((*this)[pos] == 0) {
            // carry
            this->inc(pos - 1);
        }
    }

    uint8_t _b[B];
} __attribute__((packed)); 

typedef Word<8> word64_t;

#endif /* __WORDS_H__ */