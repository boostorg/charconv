// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_CHARS_FORMAT_HPP
#define BOOST_CHARCONV_CHARS_FORMAT_HPP

#if defined(BOOST_USE_MODULES) && !defined(BOOST_CHARCONV_INTERFACE_UNIT)

#ifndef BOOST_IN_MODULE_PURVIEW
import boost.charconv;
#ifndef BOOST_CHARCONV_CONSTEXPR
#define BOOST_CHARCONV_CONSTEXPR constexpr
#endif 
#endif

#else

#include <boost/charconv/config.hpp>

namespace boost { namespace charconv {

// Floating-point format for primitive numerical conversion
// chars_format is a bitmask type (16.3.3.3.3)
BOOST_CHARCONV_MODULE_EXPORT enum class chars_format : unsigned
{
    scientific = 1 << 0,
    fixed = 1 << 1,
    hex = 1 << 2,
    general = fixed | scientific
};

}} // Namespaces

#endif // defined(BOOST_USE_MODULES) && !defined(BOOST_CHARCONV_INTERFACE_UNIT)

#endif // BOOST_CHARCONV_CHARS_FORMAT_HPP
