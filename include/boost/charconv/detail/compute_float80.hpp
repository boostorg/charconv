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
#include <boost/charconv/detail/to_chars_integer_impl.hpp>
#endif

namespace boost { namespace charconv { namespace detail {

static constexpr long double powers_of_ten_ld[] = {
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

inline uint256 compute_power_of_5(std::uint64_t n)
{
    // Use exponentiation by squaring but only capture the high bits each time
    uint256 base {0, 5};
    uint256 result {0, 1};
    while (n > 1)
    {
        if (n & 1)
        {
            result = umul512_high256(base, result);
        }
        base = umul512_high256(base, base);
        n /= 2;
    }

    return result;
}

template <typename T>
inline T huge_val() noexcept;

template <>
inline long double huge_val() noexcept
{
    return HUGE_VALL;
}

#ifdef BOOST_CHARCONV_HAS_FLOAT128
template <>
inline __float128 huge_val() noexcept
{
    return HUGE_VALQ;
}
#endif

template <typename T>
inline T zero_val() noexcept;

template <>
inline long double zero_val() noexcept
{
    return 0.0L;
}

#ifdef BOOST_CHARCONV_HAS_FLOAT128
template <>
inline __float128 zero_val() noexcept
{
    return 0.0Q;
}
#endif

template <typename T>
inline T pow2toneg113() noexcept;

template <>
inline long double pow2toneg113() noexcept
{
    return std::pow(2.0L, -113.0L);
}

#ifdef BOOST_CHARCONV_HAS_FLOAT128
template <>
inline __float128 pow2toneg113() noexcept
{
    return powq(2.0Q, -113.0Q);
}
#endif

template <typename T>
inline T pow2top(std::int64_t p) noexcept;

template <>
inline long double pow2top<long double>(std::int64_t p) noexcept
{
    return std::pow(2.0L, static_cast<long double>(p));
}

#ifdef BOOST_CHARCONV_HAS_FLOAT128
template <>
inline __float128 pow2top<__float128>(std::int64_t p) noexcept
{
    return powq(2.0Q, static_cast<__float128>(p));
}
#endif

template <typename T>
inline uint128 mask_mantissa(uint256 z) noexcept;

#if BOOST_CHARCONV_LDBL_BITS == 80

template <typename T>
inline uint128 mask_mantissa(uint256 z) noexcept
{
    static constexpr uint128 mask {0x1FFFFFFFFFFFF, UINT64_MAX};
    return (z >> 113) & mask;
}
#elif BOOST_CHARCONV_LDBL_BITS == 128

template <typename T>
inline uint128 mask_mantissa(uint256 z) noexcept
{
    static constexpr uint128 mask {0x1FFFFFFFFFFFF, UINT64_MAX};
    return (z >> 113) & mask;
}

#endif

#ifdef BOOST_CHARCONV_HAS_FLOAT128
template <>
inline uint128 mask_mantissa<__float128>(uint256 z) noexcept
{
    static constexpr uint128 mask {0x1FFFFFFFFFFFF, UINT64_MAX};
    return (z >> 113) & mask;
}

#endif

template <typename T>
inline uint128 significant_bit(uint128 z) noexcept;

#if BOOST_CHARCONV_LDBL_BITS == 80

template <typename T>
inline uint128 significant_bit(uint128 z) noexcept
{
    return z / (uint128(1) << 79);
}

#elif BOOST_CHARCONV_LDBL_BITS == 128

template <typename T>
inline uint128 significant_bit(uint128 z) noexcept
{
    return z / (uint128(1) << 127);
}

#endif

#ifdef BOOST_CHARCONV_HAS_FLOAT128
template <>
inline uint128 significant_bit<__float128>(uint128 z) noexcept
{
    return z / (uint128(1) << 127);
}
#endif


template <typename ResultType, typename Unsigned_Integer>
inline ResultType compute_float80(std::int64_t q, Unsigned_Integer w, bool negative, std::errc& success) noexcept
{
    // GLIBC uses 2^-16444 but MPFR uses 2^-16445 as the smallest subnormal value for 80 bit
    // 39 is the max number of digits in an uint128_t
    static constexpr auto smallest_power = -4951;
    static constexpr auto largest_power = 4932;
    static constexpr auto smallest_binary_power = -16444;
    static constexpr auto largest_binary_power = 16383;
    static const auto pow2to113 = (uint128(1) << 113);

    // We start with a fast path
    // It is an extension of what was described in Clinger WD.
    // How to read floating point numbers accurately.
    // ACM SIGPLAN Notices. 1990
    // https://dl.acm.org/doi/pdf/10.1145/93542.93557
    BOOST_CHARCONV_IF_CONSTEXPR (std::is_same<ResultType, long double>::value)
    {
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
            success = std::errc();
            return fast_path<ResultType>(q, w, negative, powers_of_tenq);
        }
    }
    #endif

    // Steps 1 and 2: Return now if the number is unrepresentable
    #ifdef BOOST_CHARCONV_DEBUG_FLOAT128
    char buffer [100] {};
    to_chars128(buffer, buffer+sizeof(buffer), static_cast<boost::uint128_type>(w));
    std::cerr << "\nInputs"
              << "\nMantissa: " << buffer
              << "\nPower: " << q
              << "\nSign: " << negative << std::endl;
    #endif

    if (w == 0)
    {
        success = std::errc();
        return negative ? -0.0L : 0.0L;
    }
    else if (q > largest_power)
    {
        success = std::errc::result_out_of_range;
        return negative ? -huge_val<ResultType>() : huge_val<ResultType>();
    }
    else if (q < smallest_power)
    {
        success = std::errc::result_out_of_range;
        return negative ? -zero_val<ResultType>() : zero_val<ResultType>();
    }

    // Step 3: Compute the number of leading zeros of w and store as leading_zeros
    // UB when w is 0 but this has already been filtered out in step 1
    auto leading_zeros = clz_u128(w);

    // Step 4: Normalize the significand
    w = (1 << leading_zeros) * w;

    // Step 5a: Compute the truncated 256-bit product stopping after 1 multiplication if
    // no more are required to represent the number exactly
    auto z = umul256(significand_256_high[q - smallest_power], w) / UINT64_MAX;

    // Step 5b: Some kind of branch to use the second table

    // Step 6: Abort if the number is unrepresentable
    #ifdef BOOST_CHARCONV_DEBUG_FLOAT128
    std::memset(buffer, '\0', sizeof(buffer));
    auto r = to_chars128(buffer, buffer+sizeof(buffer), static_cast<boost::uint128_type>(z.high));
    to_chars128(r.ptr, buffer+sizeof(buffer), static_cast<boost::uint128_type>(z.low));
    std::cerr << "\nz: " << buffer << std::endl;
    #endif

    if (BOOST_UNLIKELY(z % UINT64_MAX == (UINT64_MAX - 1) && (q < -27 || q > 55)))
    {
        success = std::errc::not_supported;
        return 0;
    }

    // Step 7: Capture the most significant bits (for the significand)
    auto m = mask_mantissa<ResultType>(z);
    #ifdef BOOST_CHARCONV_DEBUG_FLOAT128
    std::memset(buffer, '\0', sizeof(buffer));
    to_chars128(buffer, buffer+sizeof(buffer), static_cast<boost::uint128_type>(m));
    std::cerr << "m: " << buffer << std::endl;
    #endif

    // Step 8: Value of the most significant bit of z
    const auto u = significant_bit<ResultType>(z) == m ? 0 : 1;
    #ifdef BOOST_CHARCONV_DEBUG_FLOAT128
    std::cerr << "u: " << u << std::endl;
    #endif

    // Step 9: Calculate the expected binary exponent
    auto p = static_cast<std::int64_t>(((217706 * q) / 65536) + 63 - leading_zeros + u);
    #ifdef BOOST_CHARCONV_DEBUG_FLOAT128
    std::cerr << "p: " << p << std::endl;
    #endif

    // Step 10: Check the boundaries of the binary exponent
    if (p < smallest_binary_power - 128)
    {
        const auto s = smallest_binary_power - p + 1;
        m /= (1 << s);
        if (m & 1)
        {
            ++m;
        }
    }

    // Step 11: Line 16 rounding ties to even

    // Step 12: Round the significand
    if (m & 1)
    {
        ++m;
    }

    m /= 2;

    // Step 12: make m in range
    if (m == pow2to113)
    {
        m /= 2;
        ++p;
    }

    #ifdef BOOST_CHARCONV_DEBUG_FLOAT128
    std::memset(buffer, '\0', sizeof(buffer));
    to_chars128(buffer, buffer+sizeof(buffer), static_cast<boost::uint128_type>(m));
    std::cerr << "\nm`: " << buffer
              << "\np`: " << p << std::endl;
    #endif

    // Step 13: if the exponent is out of range than return correct HUGE_VAL
    if (p > largest_binary_power)
    {
        success = std::errc::result_out_of_range;
        return negative ? -huge_val<ResultType>() : huge_val<ResultType>();
    }

    success = std::errc();
    #ifdef BOOST_CHARCONV_DEBUG_FLOAT128
    std::cerr << "\nFinal components: "
              << "\nm_ld: " << static_cast<long double>(m)
              << "\n2^p: " << pow2top<ResultType>(p)
              << "\n2^-113: " << pow2toneg113<ResultType>() << std::endl;
    #endif

    return static_cast<long double>(m) * pow2top<ResultType>(p) * pow2toneg113<ResultType>();

    /*
    // Step 5a: Compute the truncated 256-bit product stopping after 1 multiplication
    // if the result is exact
    const uint128 factor_significand = significand_256_high[q - smallest_power];
    const std::int64_t exponent = (((152170 + 65536) * q) >> 16) + 1024 + 63; // Expected binary exponent
    uint256 product = umul256(w, factor_significand);
    uint128 low = product.low;
    uint128 high = product.high;

    // Step 5b:
    // We know that upper has at most one leading zero because
    // both i and  factor_mantissa have a leading one. This means
    // that the result is at least as large as ((1<<127)*(1<<127))/(1<<128).
    //
    // As long as the first 18 bits of "upper" are not "1", then we
    // know that we have an exact computed value for the leading
    // 115 bits because any imprecision would play out as a +1, in the worst case.
    // Having 115 bits is necessary because we need 113 bits for the mantissa,
    // but we have to have one rounding bit and, we can waste a bit if the most
    // significant bit of the product is zero.
    //
    // We expect this next branch to be rarely taken (say 1% of the time).
    // When (upper & 0x1FF) == 0x1FF, it can be common for
    // lower + i < lower to be true (proba. much higher than 1%).
    if (BOOST_UNLIKELY((high & UINT64_C(0x1FFFF)) == 0x1FFFF) && (low + w < low))
    {
        // The following can be used if using a pre-calculated table
        // The table for the low values is ~155kb
        // const uint128 factor_significand_low = significand_256_low[q - smallest_power];
        // product = umul256(w, factor_significand_low);
        product = compute_power_of_5(std::abs(q));
        //const uint128 product_low = product.low;
        const uint128 product_middle2 = product.high;
        const uint128 product_middle1 = low;
        uint128 product_high = high;
        const uint128 product_middle = product_middle1 + product_middle2;

        if (product_middle < product_middle1)
        {
            ++product_high;
        }

        // https://arxiv.org/pdf/2212.06644.pdf
        // Unneeded fallback checks

        // We want to check whether mantissa *it + i would affect the result
        if (((product_middle + 1 == 0) && ((product_high & UINT64_C(0x1FF)) == UINT64_C(0x1FF)) &&
            (product_low + w < product_low)))
        {
            success = std::errc::not_supported;
            return 0;
        }

        low = product_middle;
        high = product_high;
    }

    // The final significand should be 113 bits with a leading 1
    // We shift it so that it occupies 114 bits with a leading 1
    const uint128 upper_bit = high >> 127;
    uint128 significand = high >> static_cast<std::uint64_t>(upper_bit + 14);
    leading_zeros += static_cast<int>(1 ^ upper_bit);

    // If we have lots of trailing zeros we may fall between two values
    if (BOOST_UNLIKELY((low == 0) && ((high & 0x1FF) == 0) && ((significand & 3) == 1)))
    {
        // if significand & 1 == 1 we might need to round up
        success = std::errc::not_supported;
        return 0;
    }

    // Here the significand < (1 << 113), unless this is an overflow
    if (significand >= (uint128(1) << 113))
    {
        significand = (uint128(1) << 112);
        --leading_zeros;
    }

    significand &= ~(uint128(1) << 112);
    const std::uint64_t real_exponent = exponent - leading_zeros;

    // Check that the real_exponent is in range
    if (BOOST_UNLIKELY(real_exponent == 0))
    {
        success = std::errc::result_out_of_range;
        return 0;
    }
    else if (BOOST_UNLIKELY(real_exponent > 32766))
    {
        success = std::errc::result_out_of_range;
        BOOST_CHARCONV_IF_CONSTEXPR (std::is_same<ResultType, long double>::value)
        {
            return negative ? -HUGE_VALL : HUGE_VALL;
        }
        #ifdef BOOST_CHARCONV_HAS_FLOAT128
        else
        {
            // Suprerflous cast but needed to suppress warnings in C++11 and 14
            return negative ? -static_cast<ResultType>(HUGE_VALQ) : static_cast<ResultType>(HUGE_VALQ);
        }
        #endif
    }

    significand |= uint128(real_exponent) << 112;
    significand |= ((static_cast<uint128>(negative) << 127));

    ResultType res;
    trivial_uint128 temp {significand.high, significand.low};
    std::memcpy(&res, &temp, sizeof(ResultType));
    success = std::errc();

    return res;
    */
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
