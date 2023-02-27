// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/from_chars_result.hpp>
#include <boost/charconv/detail/from_chars_integer_impl.hpp>
#include <boost/charconv/config.hpp>
#include <boost/charconv/chars_format.hpp>

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

// floating point overloads

BOOST_CHARCONV_DECL from_chars_result from_chars(char const* first, char const* last, float& value, chars_format fmt = chars_format::general) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars(char const* first, char const* last, double& value, chars_format fmt = chars_format::general ) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars(char const* first, char const* last, long double& value, chars_format fmt = chars_format::general ) noexcept;

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
