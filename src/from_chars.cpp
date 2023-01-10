// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/from_chars.hpp>
#include <cstdlib>

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, int& value, int base)
{
    value = static_cast<int>(std::strtol(first, const_cast<char**>(&last), base));
    return { last, 0 };
}
