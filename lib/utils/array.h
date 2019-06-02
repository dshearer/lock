#ifndef __DYLAN_ARRAY_H_
#define __DYLAN_ARRAY_H_

#include <stddef.h>
#include <string.h>

#define DEF_SLICE_METHOD(_name, _slice_type, _const_qualifier) \
    template<int Start = 0, int End = L> \
    _slice_type<End - Start> _name() _const_qualifier { \
        static_assert(Start >= 0, "Bad start index"); \
        static_assert(Start <= L, "Bad start index"); \
        static_assert(End <= L, "Bad end index"); \
        static_assert(End >= Start, "Bad end index"); \
        _slice_type<End - Start> s = _slice_type<End - Start>(this->_data + Start); \
        return s; \
    } \

#define CARRAY_IMPL \
    template<int Offset = 0> \
    const uint8_t *data() const { \
        static_assert(Offset >= 0, "Bad offset"); \
        static_assert(Offset < L, "Bad offset"); \
        return this->_data + Offset; \
    } \
    \
    const uint8_t& operator[](size_t i) const { \
        return this->_data[i]; \
    } \
    \
    template<typename T> \
    const T *cast() const { \
        return (const T *) this->_data; \
    } \
    \
    constexpr static size_t size() { \
        return L; \
    } \
    \
    CArrayPtr operator&() const { \
        return CArrayPtr(this->_data, L); \
    } \
    \
    DEF_SLICE_METHOD(cslice, CSlice, const)

#define ARRAY_IMPL \
    CARRAY_IMPL \
    \
    void fill(uint8_t val) { \
        memset(this->_data, val, L); \
    } \
    \
    template<int L2> \
    void assign(CSlice<L2> data) { \
        static_assert(L2 <= L, "Bad slice length"); \
        memcpy(this->_data, data.data(), L2); \
    } \
    \
    uint8_t& operator[](size_t i) { \
        return this->_data[i]; \
    } \
    \
    DEF_SLICE_METHOD(slice, Slice, ) \
    \
    template<int Offset = 0> \
    uint8_t *data() { \
        static_assert(Offset < L, "Bad offset"); \
        static_assert(Offset >= 0, "Bad offset"); \
        return this->_data + Offset; \
    } \
    \
    template<typename T> \
    T *cast() { \
        return (T *) this->_data; \
    } \
    \
    template<int L2> \
    Slice<L - L2> operator<<(CSlice<L2> data) { \
        this->assign(data); \
        return this->slice<L2>(); \
    }

template<int L>
class Slice;

template<int L>
class Array;

/**
 * \brief Instances of this class are pointers to arrays with a length
 * known only at runtime.  They're just a way to pass an array and its
 * length in one object.
 */
class CArrayPtr
{
public:
    CArrayPtr(const void *data, size_t size): _data(data), _size(size) {}

    size_t size() const {
        return this->_size;
    }

    const void *data() const {
        return this->_data;
    }

private:
    const void *_data;
    size_t _size;
};

/** 
 * \brief A slice is a pointer to an array.  The slice itself has a fixed length
 * known at compile-time, which is hopefully less than or equal to the length of
 * the array.
 */
template<int L>
class CSlice
{
    static_assert(L >= 0, "Bad length");

public:
    CSlice() : _data(NULL) {};
    explicit CSlice(const void *data) : _data((const uint8_t *) data) {}

    CARRAY_IMPL

private:
    const uint8_t *_data;
} __attribute__((packed));

/** 
 * \brief A slice is a pointer to an array.  The slice itself has a fixed length
 * known at compile-time, which is hopefully less than or equal to the length of
 * the array.
 */
template<int L>
class Slice
{
    static_assert(L >= 0, "Bad length");

public:
    Slice() : _data(NULL) {};
    explicit Slice(void *data) : _data((uint8_t *) data) {}

    ARRAY_IMPL

    CSlice<L> konst() const {
        return CSlice<L>(this->_data);
    }

private:
    uint8_t *_data;
} __attribute__((packed));

/**
 * \brief Instances of this class are arrays with a fixed length known at compile-time.
 * They take up the same amount of space as a regular C array with the same length.
 * Their length (i.e., the number of bytes) can be gotten with the "sizeof" operator.
 */
template<int L>
class Array
{
public:
    Array(const Array& other) = delete;
    Array(Array& other) = delete;
    Array& operator=(const Array& other) = delete;
    Array& operator=(Array& other) = delete;

    ARRAY_IMPL

    CArrayPtr ptr() const {
        return CArrayPtr(this->_data, L);
    }

    Array& operator^=(uint8_t val) {
        for (size_t i = 0; i < L; ++i) {
            this->_data[i] ^= val;
        }
        return *this;
    }

    uint8_t _data[L];
} __attribute__((packed));

static_assert(sizeof(Array<10>) == 10, "Bad definition of Array");

#endif