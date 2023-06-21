// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
#define BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/bit.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <boost/charconv/detail/emulated256.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <system_error>
#include <type_traits>
#include <limits>
#include <cstdint>
#include <cmath>

namespace boost { namespace charconv { namespace detail {

static constexpr long double powers_of_ten[] = {
    1e0L,  1e1L,  1e2L,  1e3L,  1e4L,  1e5L,  1e6L,
    1e7L,  1e8L,  1e9L,  1e10L, 1e11L, 1e12L, 1e13L,
    1e14L, 1e15L, 1e16L, 1e17L, 1e18L, 1e19L, 1e20L,
    1e21L, 1e22L, 1e23L, 1e24L, 1e25L, 1e26L, 1e27L,
    1e28L, 1e29L, 1e30L, 1e31L, 1e32L, 1e33L, 1e34L,
    1e35L, 1e36L, 1e37L, 1e38L, 1e39L, 1e40L, 1e41L,
    1e42L, 1e43L, 1e44L, 1e45L, 1e46L, 1e47L, 1e48L
};

#ifdef BOOST_CHARCONV_HAS_FLOAT128
static constexpr __float128 powers_of_tenq[] = {
    1e0Q,  1e1Q,  1e2Q,  1e3Q,  1e4Q,  1e5Q,  1e6Q,
    1e7Q,  1e8Q,  1e9Q,  1e10Q, 1e11Q, 1e12Q, 1e13Q,
    1e14Q, 1e15Q, 1e16Q, 1e17Q, 1e18Q, 1e19Q, 1e20Q,
    1e21Q, 1e22Q, 1e23Q, 1e24Q, 1e25Q, 1e26Q, 1e27Q,
    1e28Q, 1e29Q, 1e30Q, 1e31Q, 1e32Q, 1e33Q, 1e34Q,
    1e35Q, 1e36Q, 1e37Q, 1e38Q, 1e39Q, 1e40Q, 1e41Q,
    1e42Q, 1e43Q, 1e44Q, 1e45Q, 1e46Q, 1e47Q, 1e48Q
};
#endif

// Notation:
// m -> binary significand
// p -> binary exponent
//
// w -> decimal significand
// q -> decimal exponent

template <typename ResultType, typename Unsigned_Integer, typename ArrayPtr>
inline long double fast_path(std::int64_t q, Unsigned_Integer w, bool negative, int& success, ArrayPtr table) noexcept
{
    // The general idea is as follows.
    // if 0 <= s <= 2^64 and if 10^0 <= p <= 10^27
    // Both s and p can be represented exactly
    // because of this s*p and s/p will produce
    // correctly rounded values

    auto ld = static_cast<ResultType>(w);

    if (q < 0)
    {
        ld /= table[-q];
    }
    else
    {
        ld *= table[q];
    }

    if (negative)
    {
        ld = -ld;
    }

    success = true;
    return ld;
}

template <typename ResultType, typename Unsigned_Integer>
inline ResultType compute_float80(std::int64_t q, Unsigned_Integer w, bool negative, int& success) noexcept
{
    // GLIBC uses 2^-16444 but MPFR uses 2^-16445 as the smallest subnormal value for 80 bit
    static constexpr auto smallest_power = -4951;
    static constexpr auto largest_power = 4931;

    // We start with a fast path
    // It is an extension of what was described in Clinger WD.
    // How to read floating point numbers accurately.
    // ACM SIGPLAN Notices. 1990
    // https://dl.acm.org/doi/pdf/10.1145/93542.93557
    BOOST_CHARCONV_IF_CONSTEXPR (std::is_same<ResultType, long double>::value)
    {
        const auto clinger_max_exp = BOOST_CHARCONV_LDBL_BITS == 80 ? 27 : 48;
        #if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
        if (0 <= q && q <= clinger_max_exp && w <= static_cast<Unsigned_Integer>(1) << 64)
        #else
        if (-clinger_max_exp <= q && q <= clinger_max_exp && w <= static_cast<Unsigned_Integer>(1) << 64)
        #endif
        {
            fast_path<ResultType>(q, w, negative, success, powers_of_ten);
        }
    }
    #ifdef BOOST_CHARCONV_HAS_FLOAT128
    else
    {
        #if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
        if (0 <= q && q <= 48 && w <= static_cast<Unsigned_Integer>(1) << 64)
        #else
        if (-48 <= q && q <= 48 && w <= static_cast<Unsigned_Integer>(1) << 64)
        #endif
        {
            fast_path<ResultType>(q, w, negative, success, powers_of_tenq);
        }
    }
    #endif

    // Steps 1 and 2: Return now if the number is unrepresentable
    if (w == 0)
    {
        success = true;
        return negative ? -0.0L : 0.0L;
    }
    else if (q < smallest_power)
    {
        success = static_cast<int>(std::errc::result_out_of_range);
        return negative ? -0.0L : 0.0L;
    }
    else if (q > largest_power)
    {
        success = static_cast<int>(std::errc::result_out_of_range);
        return negative ? -HUGE_VALL : HUGE_VALL;
    }

    // Step 3: Compute the number of leading zeros of w and store as l
    // UB when w is 0 but this has already been filtered out in step 1
    const auto l = clz_u128(w);

    // Step 4: Normalize the significand
    w = (1 << l) * w;
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
