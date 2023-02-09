// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/to_chars.hpp>
#include <limits>
#include <cstdio>
#include <cstring>

// stub implementations; to be replaced with real ones

boost::charconv::to_chars_result boost::charconv::to_chars( char* first, char* last, float value ) noexcept
{
    std::snprintf( first, last - first, "%.*g", std::numeric_limits<float>::max_digits10, value );
    return { first + std::strlen(first), 0 };
}

boost::charconv::to_chars_result boost::charconv::to_chars( char* first, char* last, double value ) noexcept
{
    std::snprintf( first, last - first, "%.*g", std::numeric_limits<double>::max_digits10, value );
    return { first + std::strlen(first), 0 };
}

boost::charconv::to_chars_result boost::charconv::to_chars( char* first, char* last, long double value ) noexcept
{
    std::snprintf( first, last - first, "%.*Lg", std::numeric_limits<long double>::max_digits10, value );
    return { first + std::strlen(first), 0 };
}
