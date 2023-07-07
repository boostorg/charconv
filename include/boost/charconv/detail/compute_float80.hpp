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
#include <boost/charconv/detail/significand_tables.hpp>
#include <system_error>
#include <type_traits>
#include <limits>
#include <cstdint>
#include <cmath>
#include <climits>
#include <cfloat>

#ifdef BOOST_CHARCONV_DEBUG_FLOAT128
#include <iostream>
#include <iomanip>
#include <boost/charconv/detail/to_chars_integer_impl.hpp>
#endif

namespace boost { namespace charconv { namespace detail {

#if BOOST_CHARCONV_LDBL_BITS > 64

static constexpr long double powers_of_ten_ld[] = {
    1e0L,  1e1L,  1e2L,  1e3L,  1e4L,  1e5L,  1e6L,
    1e7L,  1e8L,  1e9L,  1e10L, 1e11L, 1e12L, 1e13L,
    1e14L, 1e15L, 1e16L, 1e17L, 1e18L, 1e19L, 1e20L,
    1e21L, 1e22L, 1e23L, 1e24L, 1e25L, 1e26L, 1e27L,
    1e28L, 1e29L, 1e30L, 1e31L, 1e32L, 1e33L, 1e34L,
    1e35L, 1e36L, 1e37L, 1e38L, 1e39L, 1e40L, 1e41L,
    1e42L, 1e43L, 1e44L, 1e45L, 1e46L, 1e47L, 1e48L
};

static constexpr long double big_powers_of_ten_ld[] = {
    1e4932L, 1e4931L, 1e4930L, 1e4929L, 1e4928L, 1e4927L,
    1e4926L, 1e4925L, 1e4924L, 1e4923L, 1e4922L, 1e4921L,
    1e4920L, 1e4919L, 1e4918L, 1e4917L, 1e4916L, 1e4915L,
    1e4914L, 1e4913L, 1e4912L, 1e4911L, 1e4910L, 1e4909L,
    1e4908L, 1e4907L, 1e4906L, 1e4905L, 1e4904L, 1e4903L,
    1e4902L, 1e4901L, 1e4900L, 1e4899L, 1e4898L, 1e4897L,
    1e4896L, 1e4895L, 1e4894L, 1e4893L, 1e4892L, 1e4891L,
    1e4890L, 1e4889L, 1e4888L, 1e4887L, 1e4886L, 1e4885L
};

static constexpr long double small_powers_of_ten_ld[] = {
    0.0L, 1e-4950L, 1e-4949L, 1e-4948L, 1e-4947L, 1e-4946L,
    1e-4945L, 1e-4944L, 1e-4943L, 1e-4942L, 1e-4941L, 1e-4940L,
    1e-4939L, 1e-4938L, 1e-4937L, 1e-4936L, 1e-4935L, 1e-4934L,
    1e-4933L, 1e-4932L, 1e-4931L, 1e-4930L, 1e-4929L, 1e-4928L,
    1e-4927L, 1e-4926L, 1e-4925L, 1e-4924L, 1e-4923L, 1e-4922L,
    1e-4921L, 1e-4920L, 1e-4919L, 1e-4918L, 1e-4917L, 1e-4916L,
    1e-4915L, 1e-4914L, 1e-4913L, 1e-4912L, 1e-4911L, 1e-4910L,
    1e-4909L, 1e-4908L, 1e-4907L, 1e-4906L, 1e-4905L, 1e-4904L
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

static constexpr __float128 big_powers_of_ten_q[] = {
    1e4932Q, 1e4931Q, 1e4930Q, 1e4929Q, 1e4928Q, 1e4927Q,
    1e4926Q, 1e4925Q, 1e4924Q, 1e4923Q, 1e4922Q, 1e4921Q,
    1e4920Q, 1e4919Q, 1e4918Q, 1e4917Q, 1e4916Q, 1e4915Q,
    1e4914Q, 1e4913Q, 1e4912Q, 1e4911Q, 1e4910Q, 1e4909Q,
    1e4908Q, 1e4907Q, 1e4906Q, 1e4905Q, 1e4904Q, 1e4903Q,
    1e4902Q, 1e4901Q, 1e4900Q, 1e4899Q, 1e4898Q, 1e4897Q,
    1e4896Q, 1e4895Q, 1e4894Q, 1e4893Q, 1e4892Q, 1e4891Q,
    1e4890Q, 1e4889Q, 1e4888Q, 1e4887Q, 1e4886Q, 1e4885Q
};

static constexpr __float128 small_powers_of_ten_q[] = {
    0.0Q, 1e-4950Q, 1e-4949Q, 1e-4948Q, 1e-4947Q, 1e-4946Q,
    1e-4945Q, 1e-4944Q, 1e-4943Q, 1e-4942Q, 1e-4941Q, 1e-4940Q,
    1e-4939Q, 1e-4938Q, 1e-4937Q, 1e-4936Q, 1e-4935Q, 1e-4934Q,
    1e-4933Q, 1e-4932Q, 1e-4931Q, 1e-4930Q, 1e-4929Q, 1e-4928Q,
    1e-4927Q, 1e-4926Q, 1e-4925Q, 1e-4924Q, 1e-4923Q, 1e-4922Q,
    1e-4921Q, 1e-4920Q, 1e-4919Q, 1e-4918Q, 1e-4917Q, 1e-4916Q,
    1e-4915Q, 1e-4914Q, 1e-4913Q, 1e-4912Q, 1e-4911Q, 1e-4910Q,
    1e-4909Q, 1e-4908Q, 1e-4907Q, 1e-4906Q, 1e-4905Q, 1e-4904Q
};
#endif

// Notation:
// m -> binary significand
// p -> binary exponent
//
// w -> decimal significand
// q -> decimal exponent

template <typename ResultType, typename Unsigned_Integer, typename ArrayPtr>
inline long double fast_path(std::int64_t q, Unsigned_Integer w, bool negative, ArrayPtr table) noexcept
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

    return ld;
}

#ifdef BOOST_CHARCONV_HAS_FLOAT128
template <typename Unsigned_Integer>
inline __float128 compute_float128(std::int64_t q, Unsigned_Integer w, bool negative, std::errc& success) noexcept
{
    // GLIBC uses 2^-16444 but MPFR uses 2^-16445 as the smallest subnormal value for 80 bit
    // 39 is the max number of digits in an uint128_t
    // static constexpr auto smallest_power = -4951;
    static constexpr auto largest_power = 4932;

    #if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
    if (0 <= q && q <= 48 && w <= static_cast<Unsigned_Integer>(1) << 64)
    #else
    if (-48 <= q && q <= 48 && w <= static_cast<Unsigned_Integer>(1) << 64)
            #endif
    {
        success = std::errc();
        return fast_path<__float128>(q, w, negative, powers_of_tenq);
    }

    if (w == 0)
    {
        success = std::errc();
        return negative ? -0.0Q : 0.0Q;
    }
    else if (q > largest_power)
    {
        success = std::errc::result_out_of_range;
        return negative ? -HUGE_VALQ : HUGE_VALQ;
    }

    success = std::errc::not_supported;
    return 0;

    /*
    // If that does not work we calculate the power
    // and use our 128-bit emulated representation of the mantissa
    // which we know casts properly to long double
    uint128 man = w;
    auto return_val = static_cast<__float128>(man);

    if (q >= 4885)
    {
        return_val *= big_powers_of_ten_q[largest_power - q];
    }
    else if (q <= -4904)
    {
        return_val *= small_powers_of_ten_q[std::abs(smallest_power - q)];
    }
    else
    {
        return_val *= powq(10.0Q, static_cast<__float128>(q));
    }

    if (fabsq(return_val) > FLT128_MAX)
    {
        success = std::errc::result_out_of_range;
        return negative ? -0.0Q : 0.0Q;
    }

    return_val = negative ? -return_val : return_val;

    // Do we need to round?
    if (!(man & 1))
    {
        IEEEbinary128 bits;
        std::memcpy(&bits, &return_val, sizeof(return_val));
        ++bits.mantissa_l;
        std::memcpy(&return_val, &bits, sizeof(return_val));
    }

    success = std::errc();
    return return_val;
    */
}
#endif

template <typename ResultType, typename Unsigned_Integer>
inline ResultType compute_float80(std::int64_t q, Unsigned_Integer w, bool negative, std::errc& success) noexcept
{
    // GLIBC uses 2^-16444 but MPFR uses 2^-16445 as the smallest subnormal value for 80 bit
    // 39 is the max number of digits in an uint128_t
    static constexpr auto smallest_power = -4951;
    static constexpr auto largest_power = 4932;

    // We start with a fast path
    // It is an extension of what was described in Clinger WD.
    // How to read floating point numbers accurately.
    // ACM SIGPLAN Notices. 1990
    // https://dl.acm.org/doi/pdf/10.1145/93542.93557
    constexpr auto clinger_max_exp = BOOST_CHARCONV_LDBL_BITS == 80 ? 27 : 48;
    #if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
    if (0 <= q && q <= clinger_max_exp && w <= static_cast<Unsigned_Integer>(1) << 113)
    #else
    if (-clinger_max_exp <= q && q <= clinger_max_exp && w <= static_cast<Unsigned_Integer>(1) << 113)
    #endif
    {
        success = std::errc();
        return fast_path<ResultType>(q, w, negative, powers_of_ten_ld);
    }

    if (w == 0)
    {
        success = std::errc();
        return negative ? -0.0L : 0.0L;
    }
    else if (q > largest_power)
    {
        success = std::errc::result_out_of_range;
        return negative ? -HUGE_VALL : HUGE_VALL;
    }
    else if (q < smallest_power)
    {
        success = std::errc::result_out_of_range;
        return negative ? -0.0L : 0.0L;
    }

    success = std::errc::not_supported;
    return 0;

    /*
    // If that does not work we calculate the power
    // and use our 128-bit emulated representation of the mantissa
    // which we know casts properly to long double
    uint128 man = w;
    auto return_val = static_cast<ResultType>(man);

    if (q >= 4885)
    {
        return_val *= big_powers_of_ten_ld[largest_power - q];
    }
    else if (q <= -4904)
    {
        return_val *= small_powers_of_ten_ld[std::abs(smallest_power - q)];
    }
    else
    {
        return_val *= std::pow(10.0L, static_cast<long double>(q));
    }

    if (std::abs(return_val) > (std::numeric_limits<long double>::max)())
    {
        success = std::errc::result_out_of_range;
        return negative ? -0.0L : 0.0L;
    }

    return_val = negative ? -return_val : return_val;

    // Do we need to round?
    if (!(man & 1))
    {
        IEEEl2bits bits;
        std::memcpy(&bits, &return_val, sizeof(return_val));
        ++bits.mantissa_l;
        std::memcpy(&return_val, &bits, sizeof(return_val));
    }

    success = std::errc();
    return return_val;
    */
}

#endif // BOOST_CHARCONV_LDBL_BITS > 64

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
