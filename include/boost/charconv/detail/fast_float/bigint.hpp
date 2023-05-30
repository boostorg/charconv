// Copyright 2020-2023 Daniel Lemire
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// Derivative of: https://github.com/fastfloat/fast_float

#ifndef BOOST_CHARCONV_DETAIL_FAST_FLOAT_BIGINT_HPP
#define BOOST_CHARCONV_DETAIL_FAST_FLOAT_BIGINT_HPP

#include <boost/charconv/detail/fast_float/float_common.hpp>
#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <boost/core/bit.hpp>
#include <algorithm>
#include <cstdint>
#include <climits>
#include <cstring>

namespace boost { namespace charconv { namespace detail { namespace fast_float {

// the limb width: we want efficient multiplication of double the bits in
// limb, or for 64-bit limbs, at least 64-bit multiplication where we can
// extract the high and low parts efficiently. this is every 64-bit
// architecture except for sparc, which emulates 128-bit multiplication.
// we might have platforms where `CHAR_BIT` is not 8, so let's avoid
// doing `8 * sizeof(limb)`.
#if defined(BOOST_CHARCONV_FASTFLOAT_64BIT) && !defined(__sparc)

#define BOOST_CHARCONV_FASTFLOAT_64BIT 1

using limb = std::uint64_t;
constexpr std::size_t limb_bits = 64;

#else

#define FASTFLOAT_32BIT_LIMB
using limb = std::uint32_t;
constexpr std::size_t limb_bits = 32;

#endif

using limb_span = span<limb>;

// number of bits in a bigint. this needs to be at least the number
// of bits required to store the largest bigint, which is
// `log2(10**(digits + max_exp))`, or `log2(10**(767 + 342))`, or
// ~3600 bits, so we round to 4000.
constexpr std::size_t bigint_bits = 4000;
constexpr std::size_t bigint_limbs = bigint_bits / limb_bits;

// vector-like type that is allocated on the stack. the entire
// buffer is pre-allocated, and only the length changes.
template <std::uint16_t size>
struct stackvec
{
    limb data[size];
    // we never need more than 150 limbs
    std::uint16_t length{0};

    stackvec() = default;
    stackvec(const stackvec &) = delete;
    stackvec &operator=(const stackvec &) = delete;
    stackvec(stackvec &&) = delete;
    stackvec &operator=(stackvec &&other) = delete;

    // create stack vector from existing limb span.
    BOOST_CHARCONV_CXX20_CONSTEXPR stackvec(BOOST_ATTRIBUTE_UNUSED limb_span s) 
    {
        BOOST_CHARCONV_ASSERT(try_extend(s));
    }

    BOOST_CHARCONV_CXX14_CONSTEXPR limb& operator[](std::size_t index) noexcept
    {
        BOOST_CHARCONV_DEBUG_ASSERT(index < length);
        return data[index];
    }
    BOOST_CHARCONV_CXX14_CONSTEXPR const limb& operator[](std::size_t index) const noexcept
    {
        BOOST_CHARCONV_DEBUG_ASSERT(index < length);
        return data[index];
    }
    // index from the end of the container
    BOOST_CHARCONV_CXX14_CONSTEXPR const limb& rindex(std::size_t index) const noexcept 
    {
        BOOST_CHARCONV_DEBUG_ASSERT(index < length);
        std::size_t rindex = length - index - 1;
        return data[rindex];
    }

    // set the length, without bounds checking.
    BOOST_CHARCONV_CXX14_CONSTEXPR void set_len(std::size_t len) noexcept
    {
        length = static_cast<std::uint16_t>(len);
    }
    constexpr std::size_t len() const noexcept
    {
        return length;
    }
    constexpr bool is_empty() const noexcept
    {
        return length == 0;
    }
    constexpr std::size_t capacity() const noexcept
    {
        return size;
    }
    // append item to vector, without bounds checking
    BOOST_CHARCONV_CXX14_CONSTEXPR void push_unchecked(limb value) noexcept
    {
        data[length] = value;
        length++;
    }
    // append item to vector, returning if item was added
    BOOST_CHARCONV_CXX14_CONSTEXPR bool try_push(limb value) noexcept 
    {
        if (len() < capacity())
        {
            push_unchecked(value);
            return true;
        } 
       
        return false;
    }
    // add items to the vector, from a span, without bounds checking
    BOOST_CHARCONV_CXX20_CONSTEXPR void extend_unchecked(limb_span s) noexcept
    {
        limb* ptr = data + length;
        std::copy_n(s.ptr, s.len(), ptr);
        set_len(len() + s.len());
    }
    // try to add items to the vector, returning if items were added
    BOOST_CHARCONV_CXX20_CONSTEXPR bool try_extend(limb_span s) noexcept 
    {
        if (len() + s.len() <= capacity())
        {
            extend_unchecked(s);
            return true;
        }
        
        return false;
    }
    // resize the vector, without bounds checking
    // if the new size is longer than the vector, assign value to each
    // appended item.
    BOOST_CHARCONV_CXX20_CONSTEXPR
    void resize_unchecked(std::size_t new_len, limb value) noexcept 
    {
        if (new_len > len()) 
        {
            std::size_t count = new_len - len();
            limb* first = data + len();
            limb* last = first + count;
            std::fill(first, last, value);
            set_len(new_len);
        } 
        else 
        {
            set_len(new_len);
        }
    }
    // try to resize the vector, returning if the vector was resized.
    BOOST_CHARCONV_CXX20_CONSTEXPR bool try_resize(std::size_t new_len, limb value) noexcept
    {
        if (new_len > capacity()) 
        {
            return false;
        } 

        resize_unchecked(new_len, value);
        return true;
    }
    // check if any limbs are non-zero after the given index.
    // this needs to be done in reverse order, since the index
    // is relative to the most significant limbs.
    BOOST_CHARCONV_CXX14_CONSTEXPR bool nonzero(std::size_t index) const noexcept
    {
        while (index < len()) 
        {
            if (rindex(index) != 0) 
            {
                return true;
            }
            index++;
        }
        return false;
    }
    // normalize the big integer, so most-significant zero limbs are removed.
    BOOST_CHARCONV_CXX14_CONSTEXPR void normalize() noexcept 
    {
        while (len() > 0 && rindex(0) == 0) 
        {
            length--;
        }
    }
};

BOOST_FORCEINLINE BOOST_CHARCONV_CXX14_CONSTEXPR_NO_INLINE
std::uint64_t empty_hi64(bool& truncated) noexcept
{
    truncated = false;
    return 0;
}

BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR 
std::uint64_t uint64_hi64(std::uint64_t r0, bool& truncated) noexcept
{
    truncated = false;
    int shl = leading_zeroes(r0);
    return r0 << shl;
}

BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR 
std::uint64_t uint64_hi64(std::uint64_t r0, std::uint64_t r1, bool& truncated) noexcept
{
    int shl = leading_zeroes(r0);
    if (shl == 0) 
    {
        truncated = r1 != 0;
        return r0;
    }

    int shr = 64 - shl;
    truncated = (r1 << shl) != 0;
    return (r0 << shl) | (r1 >> shr);
}

BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR
std::uint64_t uint32_hi64(std::uint32_t r0, bool& truncated) noexcept 
{
    return uint64_hi64(r0, truncated);
}

BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR
std::uint64_t uint32_hi64(std::uint32_t r0, std::uint32_t r1, bool& truncated) noexcept
{
    std::uint64_t x0 = r0;
    std::uint64_t x1 = r1;
    return uint64_hi64((x0 << 32) | x1, truncated);
}

BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR
std::uint64_t uint32_hi64(std::uint32_t r0, std::uint32_t r1, std::uint32_t r2, bool& truncated) noexcept
{
    std::uint64_t x0 = r0;
    std::uint64_t x1 = r1;
    std::uint64_t x2 = r2;
    return uint64_hi64(x0, (x1 << 32) | x2, truncated);
}

// add two small integers, checking for overflow.
// we want an efficient operation. for msvc, where
// we don't have built-in intrinsics, this is still
// pretty fast.
BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR limb scalar_add(limb x, limb y, bool& overflow) noexcept
{
    limb z;
    // gcc and clang
    #if BOOST_CHARCONV_HAS_BUILTIN(__builtin_add_overflow)
    if (!BOOST_CHARCONV_IS_CONSTANT_EVALUATED(overflow)) 
    {
        overflow = __builtin_add_overflow(x, y, &z);
        return z;
    }
    #endif

    // generic, this still optimizes correctly on MSVC.
    z = x + y;
    overflow = z < x;
    return z;
}

// multiply two small integers, getting both the high and low bits.
BOOST_FORCEINLINE
#ifndef BOOST_MSVC 
BOOST_CHARCONV_CXX20_CONSTEXPR
#endif
limb scalar_mul(limb x, limb y, limb& carry) noexcept
{
    #ifdef BOOST_CHARCONV_FASTFLOAT_64BIT
    #  ifdef BOOST_CHARCONV_HAS_INT128
        
        // GCC and clang both define it as an extension.
        uint128_type z = static_cast<uint128_type>(x) * static_cast<uint128_type>(y) + static_cast<uint128_type>(carry);
        carry = limb(z >> limb_bits);
        return limb(z);

    #  else

        // fallback, no native 128-bit integer multiplication with carry.
        // on msvc, this optimizes identically, somehow.
        uint128 z = umul128(x, y);
        bool overflow;
        z.low = scalar_add(z.low, carry, overflow);
        z.high += static_cast<std::uint64_t>(overflow);  // cannot overflow
        carry = z.high;
        return z.low;

    #  endif
    #else

        std::uint64_t z = static_cast<std::uint64_t>(x) * static_cast<std::uint64_t>(y) + static_cast<std::uint64_t>(carry);
        carry = static_cast<limb>(z >> limb_bits);
        return static_cast<limb>(z);

    #endif
}

// add scalar value to bigint starting from offset.
// used in grade school multiplication
template <std::uint16_t size>
inline BOOST_CHARCONV_CXX20_CONSTEXPR bool small_add_from(stackvec<size>& vec, limb y, size_t start) noexcept 
{
    std::size_t index = start;
    limb carry = y;
    bool overflow;
    while (carry != 0 && index < vec.len())
    {
        vec[index] = scalar_add(vec[index], carry, overflow);
        carry = limb(overflow);
        index += 1;
    }
    if (carry != 0) 
    {
        BOOST_CHARCONV_TRY(vec.try_push(carry));
    }

    return true;
}

// add scalar value to bigint.
template <std::uint16_t size>
BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR bool small_add(stackvec<size>& vec, limb y) noexcept
{
    return small_add_from(vec, y, 0);
}

// multiply bigint by scalar value.
template <std::uint16_t size>
inline BOOST_CHARCONV_CXX20_CONSTEXPR bool small_mul(stackvec<size>& vec, limb y) noexcept
{
    limb carry = 0;
    for (std::size_t index = 0; index < vec.len(); index++)
    {
        vec[index] = scalar_mul(vec[index], y, carry);
    }
    if (carry != 0)
    {
        BOOST_CHARCONV_TRY(vec.try_push(carry));
    }

    return true;
}

// add bigint to bigint starting from index.
// used in grade school multiplication
template <std::uint16_t size>
BOOST_CHARCONV_CXX20_CONSTEXPR bool large_add_from(stackvec<size>& x, limb_span y, size_t start) noexcept
{
    // the effective x buffer is from `xstart..x.len()`, so exit early
    // if we can't get that current range.
    if (x.len() < start || y.len() > x.len() - start) 
    {
        BOOST_CHARCONV_TRY(x.try_resize(y.len() + start, 0));
    }

    bool carry = false;
    for (std::size_t index = 0; index < y.len(); index++)
    {
        limb xi = x[index + start];
        limb yi = y[index];
        bool c1 = false;
        bool c2 = false;
        xi = scalar_add(xi, yi, c1);
        if (carry) 
        {
            xi = scalar_add(xi, 1, c2);
        }
        x[index + start] = xi;
        carry = c1 | c2;
    }

    // handle overflow
    if (carry) 
    {
        BOOST_CHARCONV_TRY(small_add_from(x, 1, y.len() + start));
    }

    return true;
}

// add bigint to bigint.
template <std::uint16_t size>
BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR bool large_add_from(stackvec<size>& x, limb_span y) noexcept
{
    return large_add_from(x, y, 0);
}

// grade-school multiplication algorithm
template <std::uint16_t size>
BOOST_CHARCONV_CXX20_CONSTEXPR bool long_mul(stackvec<size>& x, limb_span y) noexcept 
{
    limb_span xs = limb_span(x.data, x.len());
    stackvec<size> z(xs);
    limb_span zs = limb_span(z.data, z.len());

    if (y.len() != 0)
    {
        limb y0 = y[0];
        BOOST_CHARCONV_TRY(small_mul(x, y0));
        for (size_t index = 1; index < y.len(); index++) 
        {
            limb yi = y[index];
            stackvec<size> zi;
            if (yi != 0) 
            {
                // re-use the same buffer throughout
                zi.set_len(0);
                BOOST_CHARCONV_TRY(zi.try_extend(zs));
                BOOST_CHARCONV_TRY(small_mul(zi, yi));
                limb_span zis = limb_span(zi.data, zi.len());
                BOOST_CHARCONV_TRY(large_add_from(x, zis, index));
            }
        }
    }

    x.normalize();
    return true;
}

// grade-school multiplication algorithm
template <std::uint16_t size>
BOOST_CHARCONV_CXX20_CONSTEXPR bool large_mul(stackvec<size>& x, limb_span y) noexcept
{
    if (y.len() == 1)
    {
        BOOST_CHARCONV_TRY(small_mul(x, y[0]));
    } 
    else
    {
        BOOST_CHARCONV_TRY(long_mul(x, y));
    }

    return true;
}

template <typename = void>
struct pow5_tables
{
    static constexpr std::uint32_t large_step = UINT32_C(135);
    static constexpr std::uint64_t small_power_of_5[] = {
        UINT64_C(1), UINT64_C(5), UINT64_C(25), UINT64_C(125), UINT64_C(625), UINT64_C(3125), UINT64_C(15625), UINT64_C(78125), UINT64_C(390625),
        UINT64_C(1953125), UINT64_C(9765625), UINT64_C(48828125), UINT64_C(244140625), UINT64_C(1220703125),
        UINT64_C(6103515625), UINT64_C(30517578125), UINT64_C(152587890625), UINT64_C(762939453125),
        UINT64_C(3814697265625), UINT64_C(19073486328125), UINT64_C(95367431640625), UINT64_C(476837158203125),
        UINT64_C(2384185791015625), UINT64_C(11920928955078125), UINT64_C(59604644775390625),
        UINT64_C(298023223876953125), UINT64_C(1490116119384765625), UINT64_C(7450580596923828125)
    };

    #ifdef BOOST_CHARCONV_FASTFLOAT_64BIT

    constexpr static limb large_power_of_5[] = {
        UINT64_C(1414648277510068013), UINT64_C(9180637584431281687), UINT64_C(4539964771860779200),
        UINT64_C(10482974169319127550), UINT64_C(198276706040285095)};

    #else

    constexpr static limb large_power_of_5[] = {
        UINT32_C(4279965485), UINT32_C(329373468), UINT32_C(4020270615), UINT32_C(2137533757), UINT32_C(4287402176),
        UINT32_C(1057042919), UINT32_C(1071430142), UINT32_C(2440757623), UINT32_C(381945767), UINT32_C(46164893)};

    #endif
};

#if defined(BOOST_NO_CXX17_INLINE_VARIABLES) && BOOST_MSVC != 1900
template <typename T>
constexpr uint32_t pow5_tables<T>::large_step;

template <typename T>
constexpr uint64_t pow5_tables<T>::small_power_of_5[];

template <typename T>
constexpr limb pow5_tables<T>::large_power_of_5[];
#endif

// big integer type. implements a small subset of big integer
// arithmetic, using simple algorithms since asymptotically
// faster algorithms are slower for a small number of limbs.
// all operations assume the big-integer is normalized.
struct bigint : pow5_tables<> 
{
    // storage of the limbs, in little-endian order.
    stackvec<bigint_limbs> vec;

    BOOST_CHARCONV_CXX20_CONSTEXPR bigint() : vec() {}
    bigint(const bigint &) = delete;
    bigint &operator=(const bigint &) = delete;
    bigint(bigint &&) = delete;
    bigint &operator=(bigint &&other) = delete;

    BOOST_CHARCONV_CXX20_CONSTEXPR bigint(std::uint64_t value) : vec() 
    {
        #ifdef BOOST_CHARCONV_FASTFLOAT_64BIT

        vec.push_unchecked(value);

        #else

        vec.push_unchecked(static_cast<std::uint32_t>(value));
        vec.push_unchecked(static_cast<std::uint32_t>(value >> 32));

        #endif

        vec.normalize();
    }

    // get the high 64 bits from the vector, and if bits were truncated.
    // this is to get the significant digits for the float.
    BOOST_CHARCONV_CXX20_CONSTEXPR std::uint64_t hi64(bool& truncated) const noexcept 
    {
        #ifdef BOOST_CHARCONV_FASTFLOAT_64BIT
        if (vec.len() == 0) 
        {
            return empty_hi64(truncated);
        }
        else if (vec.len() == 1) 
        {
            return uint64_hi64(vec.rindex(0), truncated);
        }
        else 
        {
            std::uint64_t result = uint64_hi64(vec.rindex(0), vec.rindex(1), truncated);
            truncated |= vec.nonzero(2);
            return result;
        }

        #else

        if (vec.len() == 0) 
        {
            return empty_hi64(truncated);
        }
        else if (vec.len() == 1) 
        {
            return uint32_hi64(vec.rindex(0), truncated);
        }
        else if (vec.len() == 2) 
        {
            return uint32_hi64(vec.rindex(0), vec.rindex(1), truncated);
        } 
        else 
        {
            std::uint64_t result = uint32_hi64(vec.rindex(0), vec.rindex(1), vec.rindex(2), truncated);
            truncated |= vec.nonzero(3);
            return result;
        }

        #endif
    }

    // compare two big integers, returning the large value.
    // assumes both are normalized. if the return value is
    // negative, other is larger, if the return value is
    // positive, this is larger, otherwise they are equal.
    // the limbs are stored in little-endian order, so we
    // must compare the limbs in ever order.
    BOOST_CHARCONV_CXX20_CONSTEXPR int compare(const bigint& other) const noexcept 
    {
        if (vec.len() > other.vec.len())
        {
            return 1;
        } 
        else if (vec.len() < other.vec.len())
        {
            return -1;
        } 

        for (std::size_t index = vec.len(); index > 0; index--) 
        {
            limb xi = vec[index - 1];
            limb yi = other.vec[index - 1];
            if (xi > yi) 
            {
                return 1;
            } 
            else if (xi < yi)
            {
                return -1;
            }
        }

        return 0;
    }

    // shift left each limb n bits, carrying over to the new limb
    // returns true if we were able to shift all the digits.
    BOOST_CHARCONV_CXX20_CONSTEXPR bool shl_bits(std::size_t n) noexcept
    {
        // Internally, for each item, we shift left by n, and add the previous
        // right shifted limb-bits.
        // For example, we transform (for u8) shifted left 2, to:
        //      b10100100 b01000010
        //      b10 b10010001 b00001000
        BOOST_CHARCONV_DEBUG_ASSERT(n != 0);
        BOOST_CHARCONV_DEBUG_ASSERT(n < sizeof(limb) * 8);

        std::size_t shl = n;
        std::size_t shr = limb_bits - shl;
        limb prev = 0;
        for (std::size_t index = 0; index < vec.len(); index++) 
        {
            limb xi = vec[index];
            vec[index] = (xi << shl) | (prev >> shr);
            prev = xi;
        }

        limb carry = prev >> shr;
        if (carry != 0) 
        {
            return vec.try_push(carry);
        }

        return true;
    }

#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4996) // Not all versions of MSVC respond as they should to _SCL_NO_WARNINGS
#endif

    // move the limbs left by `n` limbs.
    BOOST_CHARCONV_CXX20_CONSTEXPR bool shl_limbs(std::size_t n) noexcept
    {
        BOOST_CHARCONV_DEBUG_ASSERT(n != 0);

        if (n + vec.len() > vec.capacity())
        {
            return false;
        } 
        else if (!vec.is_empty()) 
        {
            // move limbs
            limb* dst = vec.data + n;
            const limb* src = vec.data;
            std::copy_backward(src, src + vec.len(), dst + vec.len());
            // fill in empty limbs
            limb* first = vec.data;
            limb* last = first + n;
            ::std::fill(first, last, 0);
            vec.set_len(n + vec.len());
            return true;
        }

        return true;
    }

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif

    // move the limbs left by `n` bits.
    BOOST_CHARCONV_CXX20_CONSTEXPR bool shl(std::size_t n) noexcept
    {
        std::size_t rem = n % limb_bits;
        std::size_t div = n / limb_bits;
        if (rem != 0) 
        {
            BOOST_CHARCONV_TRY(shl_bits(rem));
        }
        if (div != 0) 
        {
            BOOST_CHARCONV_TRY(shl_limbs(div));
        }

        return true;
    }

    // get the number of leading zeros in the bigint.
    BOOST_CHARCONV_CXX20_CONSTEXPR int ctlz() const noexcept
    {
        if (vec.is_empty()) 
        {
            return 0;
        }
        #ifdef BOOST_CHARCONV_FASTFLOAT_64BIT
        
        return leading_zeroes(vec.rindex(0));

        #else

        // no use defining a specialized leading_zeroes for a 32-bit type.
        std::uint64_t r0 = vec.rindex(0);
        return leading_zeroes(r0 << 32);

        #endif
    }

    // get the number of bits in the bigint.
    BOOST_CHARCONV_CXX20_CONSTEXPR int bit_length() const noexcept
    {
        int lz = ctlz();
        return int(limb_bits * vec.len()) - lz;
    }

    BOOST_CHARCONV_CXX20_CONSTEXPR bool mul(limb y) noexcept
    {
        return small_mul(vec, y);
    }

    BOOST_CHARCONV_CXX20_CONSTEXPR bool add(limb y) noexcept 
    {
        return small_add(vec, y);
    }

    // multiply as if by 2 raised to a power.
    BOOST_CHARCONV_CXX20_CONSTEXPR bool pow2(std::uint32_t exp) noexcept
    {
        return shl(exp);
    }

    // multiply as if by 5 raised to a power.
    BOOST_CHARCONV_CXX20_CONSTEXPR bool pow5(std::uint32_t exp) noexcept
    {
        // multiply by a power of 5
        constexpr std::size_t large_length = sizeof(large_power_of_5) / sizeof(limb);
        limb_span large = limb_span(large_power_of_5, large_length);
        while (exp >= large_step) 
        {
            BOOST_CHARCONV_TRY(large_mul(vec, large));
            exp -= large_step;
        }

        #ifdef BOOST_CHARCONV_FASTFLOAT_64BIT

        std::uint32_t small_step = 27;
        limb max_native = UINT64_C(7450580596923828125);

        #else

        std::uint32_t small_step = 13;
        limb max_native = UINT32_C(1220703125);

        #endif

        while (exp >= small_step) 
        {
            BOOST_CHARCONV_TRY(small_mul(vec, max_native));
            exp -= small_step;
        }
        if (exp != 0) 
        {
            // Work around clang bug https://godbolt.org/z/zedh7rrhc
            // This is similar to https://github.com/llvm/llvm-project/issues/47746,
            // except the workaround described there don't work here
            BOOST_CHARCONV_TRY(small_mul(vec, static_cast<limb>((static_cast<void>(small_power_of_5[0]), small_power_of_5[exp]))));
        }

        return true;
    }

    // multiply as if by 10 raised to a power.
    BOOST_CHARCONV_CXX20_CONSTEXPR bool pow10(std::uint32_t exp) noexcept
    {
        BOOST_CHARCONV_TRY(pow5(exp));
        return pow2(exp);
    }
};

}}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_FAST_FLOAT_BIGINT_HPP
