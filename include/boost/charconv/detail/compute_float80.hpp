// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
#define BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP

#include <boost/charconv/detail/config.hpp>
#include <limits>
#include <cstdint>
#include <cmath>

namespace boost { namespace charconv { namespace detail {

static constexpr long double big_powers[] = {
    1e4892L, 1e4893L, 1e4894L, 1e4895L, 1e4896L, 1e4897L, 1e4898L,
    1e4899L, 1e4900L, 1e4901L, 1e4902L, 1e4903L, 1e4904L, 1e4905L,
    1e4906L, 1e4907L, 1e4908L, 1e4909L, 1e4910L, 1e4911L, 1e4912L
};

static_assert(sizeof(big_powers) == 21*sizeof(long double), "Big powers table must have 21 elements");

inline long double compute_float80(std::int64_t power, boost::uint128_type i, bool negative, bool& success) noexcept
{
    long double return_val;

    // At the absolute minimum and maximum rounding errors of 1 ULP can cause overflow
    if (power == 4912 && static_cast<std::uint64_t>(i) == UINT64_C(8292685093465866806))
    {
        return_val = std::numeric_limits<long double>::max();
    }
    else if (power == -4952 && static_cast<std::uint64_t>(i) == UINT64_C(4168920984437421538))
    {
        return_val = std::numeric_limits<long double>::min();
    }
    else if (power >= 4892)
    {
        return_val = i * big_powers[power - 4892];
    }
    else
    {
        return_val = static_cast<long double>(i * std::pow(10.0L, static_cast<long double>(power)));
        if (std::abs(return_val) > (std::numeric_limits<long double>::max)())
        {
            success = false;
            return negative ? -0.0L : 0.0L;
        }
    }

    return_val = negative ? -return_val : return_val;

    success = true;
    return return_val;
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
