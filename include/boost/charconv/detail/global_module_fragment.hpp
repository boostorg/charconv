// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_GLOBAL_MODULE_FRAGMENT_HPP
#define BOOST_CHARCONV_DETAIL_GLOBAL_MODULE_FRAGMENT_HPP

// Includes to be placed in the global module fragment.
// Reused by tests that include headers with names
// that are not exported from the module.

#include <cstdint> // for UINT64_C
#include <climits> // for CHAR_BIT
#include <cfloat>
#include <cmath> // for HUGE_VAL
#include <cerrno>
#include <boost/config.hpp>
#include <boost/assert.hpp>

#ifdef __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if defined(BOOST_MSVC)
#include <intrin.h>
#endif

#ifdef BOOST_CHARCONV_HAS_QUADMATH
#include <quadmath.h>
#endif

import std;
import boost.core;

#endif
