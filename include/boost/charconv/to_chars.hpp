#ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED

// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/config.hpp>

namespace boost
{
namespace charconv
{

struct to_chars_result
{
    char * ptr;
    int ec;
};

BOOST_CHARCONV_DECL to_chars_result to_chars( char* first, char* last, int value, int base = 10 );

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
