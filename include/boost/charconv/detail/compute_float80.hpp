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

static constexpr long double powers_of_ten[] = {
    1e0L,  1e1L,  1e2L,  1e3L,  1e4L,  1e5L,  1e6L,
    1e7L,  1e8L,  1e9L,  1e10L, 1e11L, 1e12L, 1e13L,
    1e14L, 1e15L, 1e16L, 1e17L, 1e18L, 1e19L, 1e20L,
    1e21L, 1e22L, 1e23L, 1e24L, 1e25L, 1e26L, 1e27L
};

template <typename Unsigned_Integer>
inline long double compute_float80(std::int64_t power, Unsigned_Integer i, bool negative, bool& success) noexcept
{
    // GLIBC uses 2^-16444 but MPFR uses 2^-16445 as the smallest subnormal value for 80 bit
    static constexpr auto smallest_power = -4951;
    static constexpr auto largest_power = 4931;

    // We start with a fast path
    // It is an extension of what was described in Clinger WD.
    // How to read floating point numbers accurately.
    // ACM SIGPLAN Notices. 1990
    // https://dl.acm.org/doi/pdf/10.1145/93542.93557
    #if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
    if (0 <= power && power <= 27 && i <= static_cast<Unsigned_Integer>(1) << 64)
    #else
    if (-27 <= power && power <= 27 && i <= static_cast<Unsigned_Integer>(1) << 64)
    #endif
    {
        // The general idea is as follows.
        // if 0 <= s <= 2^64 and if 10^0 <= p <= 10^27
        // Both s and p can be represented exactly
        // because of this s*p and s/p will produce
        // correctly rounded values

        auto ld = static_cast<long double>(i);

        if (power < 0)
        {
            ld /= powers_of_ten[-power];
        }
        else
        {
            ld *= powers_of_ten[power];
        }

        if (negative)
        {
            ld = -ld;
        }

        success = true;
        return ld;
    }

    if (i == 0 || power < smallest_power)
    {
        return negative ? -0.0L : 0.0L;
    }
    else if (power > largest_power)
    {
        return negative ? -HUGE_VALL : HUGE_VALL;
    }
}

/*
inline long double compute_float80(std::int64_t power, std::uint64_t i, bool negative, bool& success) noexcept
{
    long double return_val;

    // At the absolute minimum and maximum rounding errors of 1 ULP can cause overflow
    if (power == 4914 && i == UINT64_C(1189731495357231765))
    {
        return_val = std::numeric_limits<long double>::max();
    }
    else if (power == -4950 && i == UINT64_C(3362103143112093506))
    {
        return_val = std::numeric_limits<long double>::min();
    }
    else
    {
        return_val = i * std::pow(10.0L, static_cast<long double>(power));
        if (std::isinf(return_val))
        {
            success = false;
            return negative ? -0.0L : 0.0L;
        }
    }

    return_val = negative ? -return_val : return_val;

    success = true;
    return return_val;
}
*/
}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
