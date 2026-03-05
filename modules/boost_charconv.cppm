module;

#include <cfloat> // floating-point macros
#include <version> // stdfloat macros are pulled by this header, too
#include <math.h> // math macros
#include <stdint.h> // int macros
#include <limits.h> // CHAR_BIT and other C limits macros
#include <cinttypes> // printf format constants
#include <cerrno>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/charconv/config.hpp>
#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/disable_module_warnings.hpp>

export module boost.charconv;

import std;
import std.compat;
import boost.core;

#define BOOST_CHARCONV_INTERFACE_UNIT
#define BOOST_IN_MODULE_PURVIEW

#include <boost/charconv.hpp>
#include <boost/charconv/detail/ryu/ryu_generic_128.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>

