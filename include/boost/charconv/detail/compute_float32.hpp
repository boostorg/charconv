// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT32_HPP
#define BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT32_HPP

#include <boost/charconv/detail/compute_float64.hpp>
#include <limits>
#include <cstdint>

namespace boost { namespace charconv { namespace detail {

inline float compute_float32(std::int64_t power, std::uint64_t i, bool negative, bool& success) noexcept
{
    const double d = compute_float64(power, i, negative, success);
    
    if (d > (std::numeric_limits<float>::max)() || d < (std::numeric_limits<float>::lowest)())
    {
        success = false;
        return negative ? -0.0F : 0.0F;
    }

    return static_cast<float>(d);
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT32_HPP
