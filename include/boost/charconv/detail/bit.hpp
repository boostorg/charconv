// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_BIT_HPP
#define BOOST_CHARCONV_DETAIL_BIT_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <boost/core/bit.hpp>
#include <type_traits>
#include <limits>
#include <cstdint>

// Derivative of the following SO answer:
// https://stackoverflow.com/questions/28423405/counting-the-number-of-leading-zeros-in-a-128-bit-integer

namespace boost { namespace charconv { namespace detail {

template <typename T>
inline int clz_u128(T val) noexcept
{
    static_assert(sizeof(T) == 16 && (!std::numeric_limits<T>::is_signed
                                        #ifdef BOOST_CHARCONV_HAS_INT128
                                        // May not have numeric_limits specialization without gnu mode
                                        || std::is_same<T, boost::uint128_type>::value
                                        #endif
                                     ), "This function is only for 128-bit unsigned types");

    const std::uint64_t hi = val >> 64;
    const std::uint64_t lo = val;

    if (hi != 0)
    {
        return boost::core::countl_zero(hi);
    }

    return boost::core::countl_zero(lo) + 64;
}

template <>
inline int clz_u128<uint128>(uint128 val) noexcept
{
    if (val.high != 0)
    {
        return boost::core::countl_zero(val.high);
    }

    return boost::core::countl_zero(val.low) + 64;
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_BIT_HPP
