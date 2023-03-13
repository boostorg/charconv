// Copyright 2020-2023 Daniel Lemire
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// If the architecture (e.g. ARM) does not have __int128 we need to emulate it

#ifndef BOOST_CHARCONV_DETAIL_EMULATED128_HPP
#define BOOST_CHARCONV_DETAIL_EMULATED128_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/config.hpp>
#include <cstdint>
#include <cassert>

namespace boost { namespace charconv { namespace detail {

// Emulates a __int128 using 2x 64bit ints (high and low)
struct value128
{
    std::uint64_t low;
    std::uint64_t high;

    value128& operator+=(std::uint64_t n) noexcept
    {
        #if BOOST_CHARCONV_HAS_BUILTIN(__builtin_addcll)

        unsigned long long carry {};
        low = __builtin_addcll(low, n, 0, &carry);
        high = __builtin_addcll(high, 0, carry, &carry);

        #elif BOOST_CHARCONV_HAS_BUILTIN(__builtin_ia32_addcarryx_u64)

        unsigned long long result;
        auto carry = __builtin_ia32_addcarryx_u64(0, low, n, &result);
        low = result;
        __builtin_ia32_addcarryx_u64(carry, high, 0, &result);
        high = result;
        
        #elif defined(BOOST_CHARCONV_HAS_MSVC_64BIT_INTRINSICS)

        auto carry = _addcarry_u64(0, low, n, &low);
        _addcarry_u64(carry, high, 0, &high);

        #else

        auto sum = low + n;
        high += (sum < low ? 1 : 0);
        low = sum;

        #endif

        return *this;
    }
};

#ifndef BOOST_CHARCONV_HAS_INT128
// Returns the high 64 bits of the product of two 64-bit unsigned integers.
inline std::uint64_t umul(std::uint64_t a, std::uint64_t b) noexcept
{
    std::uint64_t    a_lo = static_cast<std::uint32_t>(a);
    std::uint64_t    a_hi = a >> 32;
    std::uint64_t    b_lo = static_cast<std::uint32_t>(b);
    std::uint64_t    b_hi = b >> 32;

    std::uint64_t    a_x_b_hi =  a_hi * b_hi;
    std::uint64_t    a_x_b_mid = a_hi * b_lo;
    std::uint64_t    b_x_a_mid = b_hi * a_lo;
    std::uint64_t    a_x_b_lo =  a_lo * b_lo;

    std::uint64_t    carry_bit = ((std::uint64_t)(std::uint32_t)a_x_b_mid + 
                                  (std::uint64_t)(std::uint32_t)b_x_a_mid + (a_x_b_lo >> 32)) >> 32;

    std::uint64_t    multhi = a_x_b_hi + (a_x_b_mid >> 32) + (b_x_a_mid >> 32) + carry_bit;

    return multhi;
}

inline value128 full_multiplication(std::uint64_t v1, std::uint64_t v2) noexcept
{
    boost::charconv::detail::value128 result;
    // https://developer.arm.com/documentation/dui0802/a/A64-General-Instructions/UMULH
    #ifdef __arm__
    result.high = __umulh(v1, v2);
    result.low = v1 * v2;
    #else
    result.high = boost::charconv::detail::umul(v1, v2);
    result.low = v1 * v2;
    #endif

    return result;
}
#else
inline value128 full_multiplication(std::uint64_t v1, std::uint64_t v2) noexcept
{
    boost::charconv::detail::value128 result;
    boost::uint128_type temp = static_cast<boost::uint128_type>(v1) * v2;
    result.low = static_cast<std::uint64_t>(temp);
    result.high = static_cast<std::uint64_t>(temp >> 64);

    return result;
}
#endif

constexpr std::uint64_t umul64(std::uint32_t x, std::uint32_t y) noexcept
{
    return x * static_cast<std::uint64_t>(y);
}

inline std::uint64_t umul128_upper64(std::uint64_t x, std::uint64_t y) noexcept
{
    return full_multiplication(x, y).high;
}

// Upper 128-bits of multiplication of 64-bit and 128-bit unsinged ints
inline value128 umul192_upper128(std::uint64_t x, value128 y) noexcept
{
    auto r = full_multiplication(x, y.high);
    r += umul128_upper64(x, y.low);

    return r;
}

inline value128 umul192_lower128(std::uint64_t x, value128 y) noexcept
{
    auto high = x * y.high;
    auto high_low = full_multiplication(x, y.low);

    return {high + high_low.high, high_low.low};
}

// Upper 64-bits of a 32-bit and a 64-bit unsigned ints
inline std::uint64_t umul96_upper64(std::uint32_t x, std::uint64_t y) noexcept
{
    #if defined(BOOST_CHARCONV_HAS_INT128) || defined(__arm__)
    
    return umul128_upper64(static_cast<std::uint64_t>(x) << 32, y);
    
    #else

    auto y_high = static_cast<std::uint32_t>(y >> 32);
    auto y_low  = static_cast<std::uint32_t>(y);

    auto xy_high = umul64(x, y_high);
    auto xy_low  = umul64(x, y_low);

    return xy_high + (xy_low >> 32);

    #endif
}

constexpr std::uint64_t umul96_lower64(std::uint32_t x, std::uint64_t y) noexcept
{
    return x * y;
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_EMULATED128_HPP
