// Copyright 2026 Ruben Perez
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
module;

#include <cinttypes> // printf format constants
#include <math.h> // math macros
#include <limits.h> // CHAR_BIT and other C limits macros
#include <cerrno>
#include "quadmath_wrapper.hpp"

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/fast_float/constexpr_feature_detect.hpp>
#include <boost/config/disable_module_warnings.hpp>

module boost.charconv:helpers;

import :interface;

#define BOOST_IN_MODULE_PURVIEW
#define BOOST_CHARCONV_INTERNAL_PARTITION_UNIT

#include <boost/charconv/detail/ryu/ryu_generic_128.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <boost/charconv/detail/compute_float80.hpp>
#include <boost/charconv/detail/fallback_routines.hpp>
#include <boost/charconv/detail/significand_tables.hpp>
#include <boost/charconv/detail/compute_float64.hpp>
#include <boost/charconv/detail/issignaling.hpp>
#include <boost/charconv/detail/fast_float/fast_float.hpp>
#include <boost/charconv/detail/dragonbox/dragonbox.hpp>
#include <boost/charconv/detail/compute_float32.hpp>
#include <boost/charconv/detail/buffer_sizing.hpp>
#include "float128_impl.hpp"
