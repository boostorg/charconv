
module;

#include <math.h> // math macros
#include <stdint.h> // int macros
#include <limits.h> // CHAR_BIT and other C limits macros
#include <cerrno>
#include <boost/config.hpp>
#include <boost/charconv/config.hpp>
#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/fast_float/constexpr_feature_detect.hpp>
#include <boost/charconv/detail/disable_module_warnings.hpp>

module boost.charconv:internal;
import std;
import boost.core;
import boost.charconv;

#define BOOST_CHARCONV_INTERNAL_PARTITION_UNIT
#define BOOST_IN_MODULE_PURVIEW

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
