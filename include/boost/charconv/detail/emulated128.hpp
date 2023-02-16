// Copyright 2020-2023 Daniel Lemire
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// If the architecture (e.g. ARM) does not have __int128 we need to emulate it

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/config.hpp>
#include <cstdint>
#include <cassert>

#ifndef BOOST_CHARCONV_DETAIL_EMULATED128_HPP
#define BOOST_CHARCONV_DETAIL_EMULATED128_HPP

namespace boost { namespace charconv { namespace detail {

// Emulates a __int128 using 2x 64bit ints (high and low)
struct value128
{
    std::uint64_t low;
    std::uint64_t high;
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

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_EMULATED128_HPP
