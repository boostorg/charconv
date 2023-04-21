// Copyright 2020-2023 Junekey Jeon
// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/to_chars.hpp>
#include <cstdio>

boost::charconv::to_chars_result boost::charconv::to_chars(char* first, char* last, float value, boost::charconv::chars_format fmt, int precision) noexcept
{
    return boost::charconv::detail::to_chars_float_impl(first, last, value, fmt, precision);
}

boost::charconv::to_chars_result boost::charconv::to_chars(char* first, char* last, double value, boost::charconv::chars_format fmt, int precision) noexcept
{
    return boost::charconv::detail::to_chars_float_impl(first, last, value, fmt, precision);
}

boost::charconv::to_chars_result boost::charconv::to_chars( char* first, char* last, long double value ) noexcept
{
    std::snprintf( first, last - first, "%.*Lg", std::numeric_limits<long double>::max_digits10, value );
    return { first + std::strlen(first), 0 };
}
