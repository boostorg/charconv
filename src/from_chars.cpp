// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/from_chars.hpp>
#include <string>
#include <cstdlib>

boost::charconv::from_chars_result boost::charconv::from_chars( char const* first, char const* last, int& value, int /*base*/ )
{
    std::string tmp( first, last );
    value = std::atoi( tmp.c_str() );
    return { last, 0 };
}
