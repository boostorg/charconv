#ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED

// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/config.hpp>

namespace boost
{
namespace charconv
{

struct from_chars_result
{
    char const* ptr;
    int ec;
};

BOOST_CHARCONV_DECL from_chars_result from_chars( char const* first, char const* last, int& value, int base = 10 );

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
