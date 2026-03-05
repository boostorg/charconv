// Copyright 2026 Ruben Perez
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
module;

#include "quadmath_wrapper.hpp"
#include <math.h>
#include <cstdint>
#include <cerrno>
#include <boost/config.hpp>
#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/disable_module_warnings.hpp>


module boost.charconv;
import std;
import std.compat;
import boost.core;

#define BOOST_IN_MODULE_PURVIEW

#include "from_chars.cpp"
