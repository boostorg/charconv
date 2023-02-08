// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_LIMITS_HPP
#define BOOST_CHARCONV_LIMITS_HPP

#include <boost/charconv/config.hpp>
#include <limits>
#include <cstdint>
#include <climits>

namespace boost { namespace charconv { 

template <typename T>
struct limits;

template <>
struct limits<char>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<char>::digits10 + 2; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<char>::digits; }
};

template <>
struct limits<signed char>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<signed char>::digits10 + 2; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<signed char>::digits; }
};

template <>
struct limits<unsigned char>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<unsigned char>::digits10 + 1; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<unsigned char>::digits; }
};

template <>
struct limits<short>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<short>::digits10 + 2; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<short>::digits; }
};

template <>
struct limits<unsigned short>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<unsigned short>::digits10 + 1; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<unsigned short>::digits; }
};

template <>
struct limits<int>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<int>::digits10 + 2; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<int>::digits; }
};

template <>
struct limits<unsigned int>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<unsigned int>::digits10 + 1; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<unsigned int>::digits; }
};

template <>
struct limits<long>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<long>::digits10 + 2; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<long>::digits; }
};

template <>
struct limits<unsigned long>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<unsigned long>::digits10 + 1; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<unsigned long>::digits; }
};

template <>
struct limits<long long>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<long long>::digits10 + 2; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<long long>::digits; }
};

template <>
struct limits<unsigned long long>
{
    static constexpr int max_chars10() noexcept { return std::numeric_limits<unsigned long long>::digits10 + 1; }
    static constexpr int max_chars()   noexcept { return std::numeric_limits<unsigned long long>::digits; }
};

template <>
struct limits<float>
{
    static constexpr int max_from_chars() noexcept { return std::numeric_limits<float>::max_digits10; }
    static constexpr int max_to_chars()   noexcept { return std::numeric_limits<float>::digits10; }
};

template <>
struct limits<double>
{
    static constexpr int max_from_chars() noexcept { return std::numeric_limits<double>::max_digits10; }
    static constexpr int max_to_chars()   noexcept { return std::numeric_limits<double>::digits10; }
};

template <>
struct limits<long double>
{
    static constexpr int max_from_chars() noexcept { return std::numeric_limits<long double>::max_digits10; }
    static constexpr int max_to_chars()   noexcept { return std::numeric_limits<long double>::digits10; }
};

#ifdef BOOST_CHARCONV_HAS_INT128

// Prior to GCC 10.3 std::numeric_limits was not specialized for __int128
// We manually specify the numbers to maximize support
//
// See: https://quuxplusone.github.io/blog/2019/02/28/is-int128-integral/

template <>
struct limits<int128_t>
{
    static constexpr int max_chars10() noexcept { return 40; } // 39 digits (2^127 - 1) + 1 for sign
    static constexpr int max_chars()   noexcept { return sizeof(int128_t) * CHAR_BIT; } 
};

template <>
struct limits<uint128_t>
{
    static constexpr int max_chars10() noexcept { return 39; } // 39 digits (2^128)
    static constexpr int max_chars()   noexcept { return sizeof(uint128_t) * CHAR_BIT; }
};

#endif // BOOST_CHARCONV_HAS_INT128

}} // Namespaces

#endif // BOOST_CHARCONV_LIMITS_HPP
