// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED

#include <boost/charconv/config.hpp>

namespace boost { namespace charconv {

// 22.13.3, Primitive numerical input conversion

struct from_chars_result
{
    const char* ptr;
    int ec;
    
    friend bool operator==(const from_chars_result& lhs, const from_chars_result& rhs)
    {
        return lhs.ptr == rhs.ptr && lhs.ec == rhs.ec;
    }

    friend bool operator!=(const from_chars_result& lhs, const from_chars_result& rhs)
    {
        return !(lhs == rhs);
    }
};

BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, int& value, int base = 10);

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
