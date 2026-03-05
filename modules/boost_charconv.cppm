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
#include <boost/charconv/detail/fast_float/constexpr_feature_detect.hpp>

export module boost.charconv;

import std;
import std.compat;
import boost.core;

#define BOOST_CHARCONV_INTERFACE_UNIT
#define BOOST_IN_MODULE_PURVIEW

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 5244)
#endif

#include <boost/charconv.hpp>

// TODO: move
#include <boost/charconv/detail/compute_float80.hpp>
#include <boost/charconv/detail/ryu/generic_128.hpp>
#include <boost/charconv/detail/ryu/ryu_generic_128.hpp>
#include <boost/charconv/detail/fallback_routines.hpp>
#include <boost/charconv/detail/significand_tables.hpp>
#include <boost/charconv/detail/compute_float64.hpp>
#include <boost/charconv/detail/issignaling.hpp>
#include <boost/charconv/detail/fast_float/fast_float.hpp>
#include <boost/charconv/detail/dragonbox/dragonbox.hpp>
#include <boost/charconv/detail/compute_float32.hpp>
#include <boost/charconv/detail/buffer_sizing.hpp>
