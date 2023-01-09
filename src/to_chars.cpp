// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/to_chars.hpp>
#include <cstdio>
#include <cstring>

boost::charconv::to_chars_result boost::charconv::to_chars( char* first, char* last, int value, int /*base*/ )
{
    std::snprintf( first, last - first - 1, "%d", value );
    return { first + std::strlen( first ), std::errc() };
}
