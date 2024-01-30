// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_FALLBACK_ROUTINES_HPP
#define BOOST_FALLBACK_ROUTINES_HPP

#include <boost/charconv/detail/to_chars_integer_impl.hpp>
#include <boost/charconv/detail/dragonbox/floff.hpp>
#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/chars_format.hpp>
#include <system_error>
#include <type_traits>
#include <cstring>
#include <cstdio>

namespace boost {
namespace charconv {
namespace detail {

#ifdef BOOST_CHARCONV_HAS_FLOAT128
inline int print_val(char* first, std::size_t size, char* format, __float128 value) noexcept
{
    return quadmath_snprintf(first, size, format, value);
}
#endif

template <typename T>
inline int print_val(char* first, std::size_t size, char* format, T value) noexcept
{
    return std::snprintf(first, size, format, value);
}

template <typename T>
to_chars_result to_chars_printf_impl(char* first, char* last, T value, chars_format fmt, int precision)
{
    // v % + . + num_digits(INT_MAX) + specifier + null terminator
    // 1 + 1 + 10 + 1 + 1
    char format[14] {};
    std::memcpy(format, "%", 1); // NOLINT : No null terminator is purposeful
    std::size_t pos = 1;

    // precision of -1 is unspecified
    if (precision != -1 && fmt != chars_format::fixed)
    {
        format[pos] = '.';
        ++pos;
        const auto unsigned_precision = static_cast<std::uint32_t>(precision);
        if (unsigned_precision < 10)
        {
            boost::charconv::detail::print_1_digit(unsigned_precision, format + pos);
            ++pos;
        }
        else if (unsigned_precision < 100)
        {
            boost::charconv::detail::print_2_digits(unsigned_precision, format + pos);
            pos += 2;
        }
        else
        {
            boost::charconv::detail::to_chars_int(format + pos, format + sizeof(format), precision);
            pos = std::strlen(format);
        }
    }
    else if (fmt == chars_format::fixed)
    {
        // Force 0 decimal places
        std::memcpy(format + pos, ".0", 2); // NOLINT : No null terminator is purposeful
        pos += 2;
    }

    // Add the type identifier
    #ifdef BOOST_CHARCONV_HAS_FLOAT128
    BOOST_CHARCONV_IF_CONSTEXPR (std::is_same<T, __float128>::value || std::is_same<T, long double>::value)
    {
        format[pos] = std::is_same<T, __float128>::value ? 'Q' : 'L';
        ++pos;
    }
    #else
    BOOST_CHARCONV_IF_CONSTEXPR (std::is_same<T, long double>::value)
    {
        format[pos] = 'L';
        ++pos;
    }
    #endif

    // Add the format character
    switch (fmt)
    {
        case boost::charconv::chars_format::general:
            format[pos] = 'g';
            break;

        case boost::charconv::chars_format::scientific:
            format[pos] = 'e';
            break;

        case boost::charconv::chars_format::fixed:
            format[pos] = 'f';
            break;

        case boost::charconv::chars_format::hex:
            format[pos] = 'a';
            break;
    }

    const auto rv = print_val(first, static_cast<std::size_t>(last - first), format, value);

    if (rv <= 0)
    {
        return {last, static_cast<std::errc>(errno)};
    }

    return {first + rv, std::errc()};
}

} //namespace detail
} //namespace charconv
} //namespace boost

#endif //BOOST_FALLBACK_ROUTINES_HPP
