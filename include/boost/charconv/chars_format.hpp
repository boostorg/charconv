// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_CHARS_FORMAT_HPP
#define BOOST_CHARCONV_CHARS_FORMAT_HPP

#include <boost/charconv/detail/config.hpp>

namespace boost { namespace charconv {

// Floating-point format for primitive numerical conversion
// chars_format is a bitmask type (16.3.3.3.3)
enum chars_format : unsigned
{
    scientific = 1 << 0,
    fixed = 1 << 1,
    hex = 1 << 2,
    general = fixed | scientific
};

constexpr chars_format operator~ (chars_format fmt) noexcept
{
    return static_cast<chars_format>(~static_cast<unsigned>(fmt));
}

constexpr chars_format operator| (chars_format lhs, chars_format rhs) noexcept
{
    return static_cast<chars_format>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
}

constexpr chars_format operator& (chars_format lhs, chars_format rhs) noexcept
{
    return static_cast<chars_format>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs));
}

constexpr chars_format operator^ (chars_format lhs, chars_format rhs) noexcept
{
    return static_cast<chars_format>(static_cast<unsigned>(lhs) ^ static_cast<unsigned>(rhs));
}

// Clang 3.7+ and MSVC 14.0+ has it figured out that this is constexpr at C++11
// No version of GCC does though
#if defined(BOOST_MSVC) || defined(__clang__)
constexpr 
#else
BOOST_CHARCONV_CXX14_CONSTEXPR
#endif
chars_format operator|= (chars_format lhs, chars_format rhs) noexcept
{
    return lhs = lhs | rhs;
}

#if defined(BOOST_MSVC) || defined(__clang__)
constexpr 
#else
BOOST_CHARCONV_CXX14_CONSTEXPR
#endif
chars_format operator&= (chars_format lhs, chars_format rhs) noexcept
{
    return lhs = lhs & rhs;
}

#if defined(BOOST_MSVC) || defined(__clang__)
constexpr 
#else
BOOST_CHARCONV_CXX14_CONSTEXPR
#endif
chars_format operator^= (chars_format lhs, chars_format rhs) noexcept
{
    return lhs = lhs ^ rhs;
}

}} // Namespaces

#endif // BOOST_CHARCONV_CHARS_FROMAT_HPP
