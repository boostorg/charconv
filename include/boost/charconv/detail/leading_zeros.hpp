// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_LEADING_ZEROS_HPP
#define BOOST_CHARCONV_DETAIL_LEADING_ZEROS_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/core/bit.hpp>
#include <cstdint>

namespace boost { namespace charconv { namespace detail {

inline int leading_zeros(std::uint64_t val) noexcept
{
    BOOST_IF_CONSTEXPR (boost::core::endian::native == boost::core::endian::little)
    {
        return boost::core::countl_zero(val);
    }
    else
    {
        return boost::core::countr_zero(val);
    }
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_LEADING_ZEROS_HPP
