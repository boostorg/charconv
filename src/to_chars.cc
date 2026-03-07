// Copyright 2026 Ruben Perez
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
module;

#include <math.h>
#include <cstdint>
#include <climits>
#include <cerrno>
#include "quadmath_wrapper.hpp"
#include <boost/config.hpp>
#include <boost/charconv/config.hpp>
#include <boost/charconv/detail/config.hpp>
#include <boost/config/disable_module_warnings.hpp>

module boost.charconv;

import :helpers;
import std;
import boost.core;

#define BOOST_IN_MODULE_PURVIEW

#include "to_chars.cpp"
