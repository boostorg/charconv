// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/from_chars.hpp>
#include <cstdio>
#include <string>
#include <stdexcept>
#include <cerrno>

#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

// stub implementations; to be replaced with real ones

boost::charconv::from_chars_result boost::charconv::from_chars( char const* first, char const* last, float& value ) noexcept
{
    from_chars_result r = {};

    std::string tmp( first, last ); // zero termination
    char* ptr = 0;

    value = std::strtof( tmp.c_str(), &ptr );

    r.ptr = ptr;
    r.ec = errno;

    return r;
}

boost::charconv::from_chars_result boost::charconv::from_chars( char const* first, char const* last, double& value ) noexcept
{
    from_chars_result r = {};

    std::string tmp( first, last ); // zero termination
    char* ptr = 0;

    value = std::strtod( tmp.c_str(), &ptr );

    r.ptr = ptr;
    r.ec = errno;

    return r;
}

boost::charconv::from_chars_result boost::charconv::from_chars( char const* first, char const* last, long double& value ) noexcept
{
    from_chars_result r = {};

    std::string tmp( first, last ); // zero termination
    char* ptr = 0;

    value = std::strtold( tmp.c_str(), &ptr );

    r.ptr = ptr;
    r.ec = errno;

    return r;
}
