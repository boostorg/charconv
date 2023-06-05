// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/from_chars_result.hpp>
#include <boost/charconv/detail/from_chars_integer_impl.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <boost/charconv/config.hpp>
#include <boost/charconv/chars_format.hpp>
#include <system_error>

namespace boost { namespace charconv {

// integer overloads

BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, bool& value, int base = 10) noexcept = delete;
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, char& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, signed char& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned char& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, short& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned short& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, int& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned int& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, long long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned long long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}

#ifdef BOOST_CHARCONV_HAS_INT128
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, boost::int128_type& value, int base = 10) noexcept
{
    return detail::from_chars_integer_impl<boost::int128_type, boost::uint128_type>(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, boost::uint128_type& value, int base = 10) noexcept
{
    return detail::from_chars_integer_impl<boost::uint128_type, boost::uint128_type>(first, last, value, base);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
// Floating Point
//----------------------------------------------------------------------------------------------------------------------

namespace detail {

std::errc errno_to_errc(int errno_value) noexcept;

} // Namespace detail

// Only 64 bit long double overloads are fully implemented
#if BOOST_CHARCONV_LDBL_BITS == 64 || defined(BOOST_MSVC)
#define BOOST_CHARCONV_FULL_LONG_DOUBLE_IMPL
#endif

BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, float& value, chars_format fmt = chars_format::general) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, double& value, chars_format fmt = chars_format::general) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, long double& value, chars_format fmt = chars_format::general) noexcept;

// <stdfloat> types
#ifdef BOOST_CHARCONV_HAS_FLOAT32
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, std::float32_t& value, chars_format fmt = chars_format::general) noexcept;
#endif
#ifdef BOOST_CHARCONV_HAS_FLOAT64
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, std::float64_t& value, chars_format fmt = chars_format::general) noexcept;
#endif

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
