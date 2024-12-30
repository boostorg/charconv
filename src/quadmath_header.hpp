// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_QUADMATH_HPP
#define BOOST_CHARCONV_QUADMATH_HPP

// Conditionally includes quadmath headers.
// Reduces duplication in modular builds
#ifdef BOOST_CHARCONV_HAS_QUADMATH
#include <quadmath.h>
#endif

#endif
