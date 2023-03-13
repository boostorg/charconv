// Copyright 2020-2023 Junekey Jeon
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_LOG_HPP
#define BOOST_CHARCONV_DETAIL_LOG_HPP

#include <boost/charconv/detail/config.hpp>
#include <cstdint>
#include <cstddef>

namespace boost { namespace charconv { namespace detail {

BOOST_CHARCONV_CXX14_CONSTEXPR std::int32_t floor_shift(std::uint32_t integer_part, std::uint64_t fractional_digits, 
                                                        std::size_t shift_amount) noexcept
{
    BOOST_CHARCONV_ASSERT(shift_amount < 32);
    // Ensure no overflow
    BOOST_CHARCONV_ASSERT(shift_amount == 0 || integer_part < (std::uint32_t(1) << (32 - shift_amount)));

    return shift_amount == 0 ? static_cast<std::int32_t>(integer_part) :
        static_cast<std::int32_t>((integer_part << shift_amount) | fractional_digits >> (64 - shift_amount));
}

// Computes floor(e * c - s)
template <
    std::uint32_t c_integer_part,
    std::uint64_t c_fractional_digits,
    std::size_t shift_amount,
    std::int32_t max_exponent,
    std::uint32_t s_integer_part = 0,
    std::uint64_t s_fractional_digits = 0
>
BOOST_CHARCONV_CXX14_CONSTEXPR int compute(int e) noexcept 
{
    BOOST_CHARCONV_ASSERT(e <= max_exponent && e >= -max_exponent);
    
    auto c = floor_shift(c_integer_part, c_fractional_digits, shift_amount);
    auto s = floor_shift(s_integer_part, s_fractional_digits, shift_amount);
    
    return (e * c - s) >> shift_amount;
}

// static constexpr std::uint64_t log10_2_fractional_digits{ 0x4d10'4d42'7de7'fbcc };
static constexpr std::uint64_t log10_2_fractional_digits = UINT64_C(5553023288523357132);
// static constexpr std::uint64_t log10_4_over_3_fractional_digits{ 0x1ffb'fc2b'bc78'0375 };
static constexpr std::uint64_t log10_4_over_3_fractional_digits = UINT64_C(2304712899105915765);
static constexpr std::size_t   floor_log10_pow2_shift_amount = 22;
static constexpr int           floor_log10_pow2_input_limit = 1700;
static constexpr int           floor_log10_pow2_minus_log10_4_over_3_input_limit = 1700;

// static constexpr std::uint64_t log10_5_fractional_digits{ 0xb2ef'b2bd'8218'0433 };
static constexpr std::uint64_t log10_5_fractional_digits = UINT64_C(12893720785186194483);
static constexpr std::size_t   floor_log10_pow5_shift_amount = 20;
static constexpr int           floor_log10_pow5_input_limit = 2620;

// static constexpr std::uint64_t log2_10_fractional_digits{ 0x5269'e12f'346e'2bf9 };
static constexpr std::uint64_t log2_10_fractional_digits = UINT64_C(5938525176524057593);
static constexpr std::size_t   floor_log2_pow10_shift_amount = 19;
static constexpr int           floor_log2_pow5_input_limit = 1764;
static constexpr int           floor_log2_pow10_input_limit = 1233;

//static constexpr std::uint64_t log5_2_fractional_digits{ 0x6e40'd1a4'143d'cb94 };
static constexpr std::uint64_t log5_2_fractional_digits = UINT64_C(7944580245325990804);
//static constexpr std::uint64_t log5_3_fractional_digits{ 0xaebf'4791'5d44'3b24 };
static constexpr std::uint64_t log5_3_fractional_digits = UINT64_C(12591861772811778852);
static constexpr std::size_t   floor_log5_pow2_shift_amount = 20;
static constexpr int           floor_log5_pow2_input_limit = 1492;
static constexpr int           floor_log5_pow2_minus_log5_3_input_limit = 2427;

// For constexpr computation
// Returns -1 when n = 0
template <typename Unsigned_Integer>
BOOST_CXX14_CONSTEXPR int floor_log2(Unsigned_Integer n) noexcept 
{
    int count = -1;
    while (n != 0)
    {
        ++count;
        n >>= 1;
    }
    return count;
}

BOOST_CXX14_CONSTEXPR int floor_log10_pow2(int e) noexcept 
{
    return compute<
        0, log10_2_fractional_digits,
        floor_log10_pow2_shift_amount,
        floor_log10_pow2_input_limit>(e);
}

BOOST_CXX14_CONSTEXPR int floor_log10_pow5(int e) noexcept
{
    return compute<
        0, log10_5_fractional_digits,
        floor_log10_pow5_shift_amount,
        floor_log10_pow5_input_limit>(e);
}

BOOST_CXX14_CONSTEXPR int floor_log2_pow5(int e) noexcept 
{
    return compute<
        2, log2_10_fractional_digits,
        floor_log2_pow10_shift_amount,
        floor_log2_pow5_input_limit>(e);
}

BOOST_CXX14_CONSTEXPR int floor_log2_pow10(int e) noexcept 
{
    return compute<
        3, log2_10_fractional_digits,
        floor_log2_pow10_shift_amount,
        floor_log2_pow10_input_limit>(e);
}

BOOST_CXX14_CONSTEXPR int floor_log5_pow2(int e) noexcept 
{
    return compute<
        0, log5_2_fractional_digits,
        floor_log5_pow2_shift_amount,
        floor_log5_pow2_input_limit>(e);
}

BOOST_CXX14_CONSTEXPR int floor_log5_pow2_minus_log5_3(int e) noexcept 
{
    return compute<
        0, log5_2_fractional_digits,
        floor_log5_pow2_shift_amount,
        floor_log5_pow2_minus_log5_3_input_limit,
        0, log5_3_fractional_digits>(e);
}

BOOST_CXX14_CONSTEXPR int floor_log10_pow2_minus_log10_4_over_3(int e) noexcept 
{
    return compute<
        0, log10_2_fractional_digits,
        floor_log10_pow2_shift_amount,
        floor_log10_pow2_minus_log10_4_over_3_input_limit,
        0, log10_4_over_3_fractional_digits>(e);
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_LOG_HPP
