// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_EMULATED256_HPP
#define BOOST_CHARCONV_DETAIL_EMULATED256_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <cstdint>

namespace boost { namespace charconv { namespace detail {

struct uint256
{
    uint128 high;
    uint128 low;
};

// Get the 256-bit result of multiplication of two 128-bit unsigned integers
inline uint256 umul256_impl(std::uint64_t a, std::uint64_t b, std::uint64_t c, std::uint64_t d) noexcept
{
    const auto ac = umul128(a, c);
    const auto bc = umul128(b, c);
    const auto ad = umul128(a, d);
    const auto bd = umul128(b, d);

    const auto intermediate = (bd >> 64) + static_cast<std::uint64_t>(ad) + static_cast<std::uint64_t>(bc);

    return {ac + (intermediate >> 64) + (ad >> 64) + (bc >> 64),
            (intermediate << 64) + static_cast<std::uint64_t>(bd)};
}

template <typename T>
inline uint256 umul256(const T& x, const T& y) noexcept
{
    static_assert(sizeof(T) == 16 && (!std::numeric_limits<T>::is_signed
                                      #ifdef BOOST_CHARCONV_HAS_INT128
                                      // May not have numeric_limits specialization without gnu mode
                                      || std::is_same<T, boost::uint128_type>::value
                                      #endif
                                      ), "This function is only for 128-bit unsigned types");

    const auto a = static_cast<std::uint64_t>(x >> 64);
    const auto b = static_cast<std::uint64_t>(x);
    const auto c = static_cast<std::uint64_t>(y >> 64);
    const auto d = static_cast<std::uint64_t>(y);

    return umul256_impl(a, b, c, d);
}

template <>
inline uint256 umul256<uint128>(const uint128& x, const uint128& y) noexcept
{
    return umul256_impl(x.high, x.low, y.high, y.low);
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_EMULATED256_HPP
