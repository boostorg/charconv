// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT32_HPP
#define BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT32_HPP

#include <boost/charconv/detail/compute_float64.hpp>
#include <cstdint>
#include <cmath>

namespace boost { namespace charconv { namespace detail {

inline float compute_float32(std::int64_t power, std::uint64_t i, bool negative, bool& success) noexcept
{
    const double d = compute_float64(power, i, negative, success);
    float return_val;

    if (success)
    {
        return_val = static_cast<float>(d);
        if (std::isinf(return_val))
        {
            return_val = negative ? -0.0F : 0.0F;
        }
    }
    else
    {
        return_val = negative ? -0.0F : 0.0F;
    }

    return return_val;
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT32_HPP
