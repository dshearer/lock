/*

Reference: https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf
Guide: https://github.com/rhash/RHash/blob/master/librhash/sha3.c
*/

#include <string.h>
#include "sha3.h"
#include "utils.h"
#include "words.h"

#define PARAM_B 1600
#define PARAM_W 64
#define PARAM_L 6

#define STATE_LEN_WORDS (PARAM_B/PARAM_W) // 25

#define NUM_ROUNDS 24

#define NUM_ELEMS(_array) (sizeof(_array)/sizeof((_array)[0]))

word64_t make_word64(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,
    uint8_t b5, uint8_t b6, uint8_t b7)
{
    word64_t w;
    w[0] = b0;
    w[1] = b1;
    w[2] = b2;
    w[3] = b3;
    w[4] = b4;
    w[5] = b5;
    w[6] = b6;
    w[7] = b7;
    return w;
}

// class word64_t
// {
// public:
//     word64_t()
//     {
//         this->zero_out();
//     }



//     void zero_out()
//     {
//         memset(this->_b, 0, sizeof(this->_b));
//     }

//     size_t size() const
//     {
//         return sizeof(this->_b);
//     }

//     const uint8_t& operator[](size_t idx) const
//     {
//         // ASSERT(idx < this->size());
//         return this->_b[idx];
//     }

//     uint8_t& operator[](size_t idx)
//     {
//         // ASSERT(idx < this->size());
//         return this->_b[idx];
//     }

//     word64_t& operator^=(const word64_t& other)
//     {
//         for (size_t i = 0; i < this->size(); ++i)
//         {
//             (*this)[i] ^= other[i];
//         }
//         return *this;
//     }

//     void rotl(size_t n)
//     {
//         this->flip();
//         for (size_t i = 0; i < n; ++i)
//         {
//             this->rotl_1();
//         }
//         this->flip();
//     }

//     void flip()
//     {
//         const size_t sz = this->size();
//         word64_t tmp;
//         for (size_t i = 0; i < sz; ++i)
//         {
//             tmp[i] = (*this)[sz - i - 1];
//         }
//         *this = tmp;
//     }

// private:
//     void rotl_1()
//     {
//         const bool last = (*this)[0] >> 7;
//         for (size_t i = 0; i < this->size(); ++i)
//         {
//             // shift byte
//             (*this)[i] <<= 1;

//             // set byte's last bit
//             bool curr_last = last;
//             if (i < this->size() - 1) {
//                 curr_last = (*this)[i + 1] >> 7;
//             }
//             set_bit(&(*this)[i], 0, curr_last);
//         }
//     }

//     uint8_t _b[8];
// } __attribute__((packed)); // big-endian

#define WORD_LEN_BITS (8 * sizeof(word64_t))

/* SHA3 (Keccak) constants for 24 rounds.

Computed with this Python script:

    n_r = 24
    L = 6

    def rc(t):
        if (t%255) == 0:
            return 1

        R = 1 << 7
        for i in xrange(1, (t%255) + 1):
            r8 = R & 1
            tmp = (r8 << 8) | (r8 << 4) | (r8 << 3) | (r8 << 2)
            R ^= tmp
            R >>= 1

        return (R & (1 << 7)) >> 7

    def set_bit(word, bit, v):
        if v:
            return word | (1 << bit)
        else:
            return word & ~(1 << bit)

    def all_rc(i_r):
        RC = 0
        for j in xrange(0, L + 1):
            bit = 2**j - 1
            val = rc(j + 7*i_r)
            RC = set_bit(RC, bit, val)
        return RC

    def print_as_word64(n):
        h = hex(n)[2:]
        if h.endswith('L'):
            h = h[:-1]

        # pad left with 0s
        assert 16 >= len(h)
        rem = 16 - len(h)
        h = ('0' * rem) + h

        parts = []
        for i in xrange(0, 16, 2):
            parts.append('0x' + h[i:i+2])

        s = ', '.join(parts)
        print '{{' + s + '}},'

    for i_r in xrange(12 + 2*L - n_r, 12 + 2*L):
        RC = all_rc(i_r)
        print_as_word64(RC)
*/
static const word64_t g_round_constants[NUM_ROUNDS] = {
    make_word64(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x82),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x8a),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x8b),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x81),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x09),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8a),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x09),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x0a),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x8b),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8b),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x89),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x02),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0a),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x0a),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x81),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80),
    make_word64(0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01),
    make_word64(0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x08),
};

static word64_t rotl64(word64_t word, size_t n)
{
    word.rotl(n);
    return word;
}

// static void xor64_inline(word64_t *w1, word64_t w2)
// {
//     return; // XXX
//     ASSERT(sizeof(*w1) == 8);
//     ASSERT(sizeof(*w1) == NUM_ELEMS(w1->b));

//     for (size_t i = 0; i < sizeof(w1->b); ++i)
//     {
//         w1->b[i] ^= w2.b[i];
//     }
// }

// static word64_t xor64(word64_t w1, word64_t w2)
// {
//     xor64_inline(&w1, w2);
//     return w1;
// }

template<size_t N>
class WordArray
{
public:
    WordArray()
    {
        memset(this->_b, 0, sizeof(this->_b));
    }

    word64_t& operator[](size_t idx)
    {
        ASSERT(NUM_ELEMS(this->_b) == N);
        if (idx >= NUM_ELEMS(this->_b)) {
            Serial.println(idx);
            ASSERT(idx < NUM_ELEMS(this->_b));
        }
        return this->_b[idx];
    }

    const word64_t& operator[](size_t idx) const
    {
        ASSERT(NUM_ELEMS(this->_b) == N);
        if (idx >= NUM_ELEMS(this->_b)) {
            Serial.println(idx);
            ASSERT(idx < NUM_ELEMS(this->_b));
        }
        return this->_b[idx];
    }

    const void *read(size_t len) const
    {
        ASSERT(len <= sizeof(this->_b));
        return this->_b;
    }

private:
    word64_t _b[N];
};

typedef WordArray<STATE_LEN_WORDS> StateWords;

static word64_t xored_a(const StateWords& A, size_t i)
{
    // return A[i] ^ A[i + 5] ^ A[i + 10] ^ A[i + 15] ^ A[i + 20];

    word64_t B = A[i];
    B ^= A[i + 5];
    B ^= A[i + 10];
    B ^= A[i + 15];
    B ^= A[i + 20];
    // xor64_inline(&B, A[i + 5]);
    // xor64_inline(&B, A[i + 10]);
    // xor64_inline(&B, A[i + 15]);
    // xor64_inline(&B, A[i + 20]);
    return B;
}

static void theta_step(StateWords& A, const WordArray<5>& D, size_t i)
{
    A[i] ^= D[i];
    A[i + 5] ^= D[i];
    A[i + 10] ^= D[i];
    A[i + 15] ^= D[i];
    A[i + 20] ^= D[i];
    // xor64_inline(&A[i],       D[i]);
    // xor64_inline(&A[i +  5],  D[i]);
    // xor64_inline(&A[i + 10],  D[i]);
    // xor64_inline(&A[i + 15],  D[i]);
    // xor64_inline(&A[i + 20],  D[i]);
}

static void theta(StateWords& A)
{
    WordArray<5> D;
    // D[0] = xor64(rotl64(xored_a(A, 1), 1), xored_a(A, 4));
    // D[1] = xor64(rotl64(xored_a(A, 2), 1), xored_a(A, 0));
    // D[2] = xor64(rotl64(xored_a(A, 3), 1), xored_a(A, 1));
    // D[3] = xor64(rotl64(xored_a(A, 4), 1), xored_a(A, 2));
    // D[4] = xor64(rotl64(xored_a(A, 0), 1), xored_a(A, 3));

    D[0] = rotl64(xored_a(A, 1), 1) ^ xored_a(A, 4);
    D[1] = rotl64(xored_a(A, 2), 1) ^ xored_a(A, 0);
    D[2] = rotl64(xored_a(A, 3), 1) ^ xored_a(A, 1);
    D[3] = rotl64(xored_a(A, 4), 1) ^ xored_a(A, 2);
    D[4] = rotl64(xored_a(A, 0), 1) ^ xored_a(A, 3);

    theta_step(A, D, 0);
	theta_step(A, D, 1);
	theta_step(A, D, 2);
	theta_step(A, D, 3);
	theta_step(A, D, 4);
}

static void rho(StateWords& A)
{
    unsigned rotations[] = {
         1, 62, 28, 27, 36, 44,  6, 55, 
        20,  3, 10, 43, 25, 39, 41, 45,
        15, 21,  8, 18,  2, 61, 56, 14,
    };

    for (size_t i = 0; i < NUM_ELEMS(rotations); ++i)
    {
        A[i + 1].rotl(rotations[i]);
    }
}

static void pi(StateWords& A)
{
    const word64_t A1 = A[1];
	A[ 1] = A[ 6];
	A[ 6] = A[ 9];
	A[ 9] = A[22];
	A[22] = A[14];
	A[14] = A[20];
	A[20] = A[ 2];
	A[ 2] = A[12];
	A[12] = A[13];
	A[13] = A[19];
	A[19] = A[23];
	A[23] = A[15];
	A[15] = A[ 4];
	A[ 4] = A[24];
	A[24] = A[21];
	A[21] = A[ 8];
	A[ 8] = A[16];
	A[16] = A[ 5];
	A[ 5] = A[ 3];
	A[ 3] = A[18];
	A[18] = A[17];
	A[17] = A[11];
	A[11] = A[ 7];
	A[ 7] = A[10];
	A[10] = A1;
}

static void chi_step(StateWords& A, size_t i)
{
	const word64_t A0 = A[0 + i];
	const word64_t A1 = A[1 + i];
    // xor64_inline(
    //     &A[0 + i],
    //     and64(not64(A1), A[2 + i])
    // );
    // xor64_inline(
    //     &A[1 + i],
    //     and64(not64(A[2 + i]), A[3 + i])
    // );
    // xor64_inline(
    //     &A[2 + i],
    //     and64(not64(A[3 + i]), A[4 + i])
    // );
    // xor64_inline(
    //     &A[3 + i],
    //     and64(not64(A[4 + i]), A0)
    // );
    // xor64_inline(
    //     &A[4 + i],
    //     and64(not64(A0), A1)
    // );
    A[0 + i] ^= ~A1         & A[2 + i];
    A[1 + i] ^= ~A[2 + i]   & A[3 + i];
    A[2 + i] ^= ~A[3 + i]   & A[4 + i];
    A[3 + i] ^= ~A[4 + i]   & A0;
    A[4 + i] ^= ~A0         & A1;
}

static void chi(StateWords& A)
{
	chi_step(A, 0);
	chi_step(A, 5);
	chi_step(A, 10);
	chi_step(A, 15);
	chi_step(A, 20);
}

static void iota(StateWords& A, unsigned r)
{
    ASSERT(r < NUM_ELEMS(g_round_constants));
    word64_t round_const = g_round_constants[r];
    round_const.flip();
    // xor64_inline(&A[0], round_const); // something wrong with &A[0]
    A[0] ^= round_const;
}

static void do_round(StateWords& A, unsigned r)
{
    #ifdef LOCAL_TEST
    const unsigned p_r = -1;
    #endif

    theta(A);
    #ifdef LOCAL_TEST
    if (r == p_r) print_hex(A, STATE_LEN_WORDS * sizeof(*A));
    #endif
    rho(A);
    #ifdef LOCAL_TEST
    if (r == p_r) print_hex(A, STATE_LEN_WORDS * sizeof(*A));
    #endif
    pi(A);
    #ifdef LOCAL_TEST
    if (r == p_r) print_hex(A, STATE_LEN_WORDS * sizeof(*A));
    #endif
    chi(A);
    #ifdef LOCAL_TEST
    if (r == p_r) print_hex(A, STATE_LEN_WORDS * sizeof(*A));
    #endif
    iota(A, r);
    #ifdef LOCAL_TEST
    if (r == p_r) print_hex(A, STATE_LEN_WORDS * sizeof(*A));
    #endif
}

/*
input must contain PARAM_B bits.
output must have space for PARAM_B bits.
*/
static void keccak_p(StateWords& A)
{
    // do rounds
    for (unsigned r = 0; r < NUM_ROUNDS; ++r)
    {
        do_round(A, r);
    }
}

class State
{
public:
    StateWords      s; // 25*8 = 200
    word64_t        curr_word;
    unsigned        curr_word_idx;
    unsigned        state_idx;
    unsigned        cap_words;

    State()
    : s(), curr_word(), curr_word_idx(0), state_idx(0), cap_words(0) {}
};

#ifdef LOCAL_TEST
static State *g_state = NULL;
#else
static State g_static_state;
static State *g_state = &g_static_state;
#endif

void sha3::init(sha3_inst_t inst)
{
    ASSERT(((unsigned) inst / 8) <= sizeof(g_state->s));

    #ifdef LOCAL_TEST
    g_state = new state_t();
    #endif
    memset(g_state, 0, sizeof(*g_state));
    g_state->cap_words = 2 * inst / WORD_LEN_BITS;
}

void sha3::update(const void *input, size_t len)
{
    const unsigned rate_words = STATE_LEN_WORDS - g_state->cap_words;

    uint8_t *input_ptr = (uint8_t *) input;
    for (size_t i = 0; i < len; ++i)
    {
        // save current byte
        g_state->curr_word[g_state->curr_word_idx] = input_ptr[i];
        ++g_state->curr_word_idx;

        if (g_state->curr_word_idx < 8) {
            continue;
        }

        // absorb current word into state
        // xor64_inline(&g_state->s[g_state->state_idx], g_state->curr_word);
        g_state->s[g_state->state_idx] ^= g_state->curr_word;
        memset(&g_state->curr_word, 0, sizeof(g_state->curr_word));
        g_state->curr_word_idx = 0;
        ++g_state->state_idx;

        if (g_state->state_idx < rate_words) {
            continue;
        }

        // permute state with Keccak
        keccak_p(g_state->s);
        g_state->state_idx = 0;
    }
}

const void *sha3::finalize(unsigned *len)
{
    // xor64_inline(&g_state->s[g_state->state_idx], g_state->curr_word);
    g_state->s[g_state->state_idx] ^= g_state->curr_word;
    g_state->s[g_state->state_idx][g_state->curr_word_idx] ^= 
        g_state->curr_word[g_state->curr_word_idx] ^ 0x06;
    g_state->s[STATE_LEN_WORDS - g_state->cap_words - 1][7] |= 0x80;
    keccak_p(g_state->s);

    const unsigned inst = g_state->cap_words * WORD_LEN_BITS / 2;
    const unsigned len_bytes = inst / 8;
    if (len != NULL) {
        *len = len_bytes;
    }
    return g_state->s.read(len_bytes);
}