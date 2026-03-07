// Copyright 2026 Ruben Perez
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

module;

#include <cfloat> // floating-point macros
#include <version> // stdfloat macros are pulled by this header, too
#include <stdint.h> // int macros
#include <limits.h> // CHAR_BIT and other C limits macros
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/charconv/config.hpp>
#include <boost/charconv/detail/config.hpp>
#include <boost/config/disable_module_warnings.hpp>

export module boost.charconv:interface;

import std;
import std.compat;
import boost.core;

#define BOOST_CHARCONV_INTERFACE_UNIT
#define BOOST_IN_MODULE_PURVIEW

#include <boost/charconv.hpp>
