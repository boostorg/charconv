// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_CONFIG_HPP
#define BOOST_CHARCONV_DETAIL_CONFIG_HPP

#include <boost/config.hpp>
#include <new>

// TODO: BOOST_ASSERT is currently unused. 
// Once library is complete remove this block, and Boost.Assert from the CML if still unused.
#ifndef BOOST_CHARCONV_STANDALONE
#  include <boost/assert.hpp>
#  define BOOST_CHARCONV_ASSERT(expr) BOOST_ASSERT(expr)
#  define BOOST_CHARCONV_ASSERT_MSG(expr, msg) BOOST_ASSERT_MSG(expr, msg)
#else // Use plain asserts
#  include <cassert>
#  define BOOST_CHARCONV_ASSERT(expr) assert(expr)
#  define BOOST_CHARCONV_ASSERT_MSG(expr, msg) assert((expr)&&(msg))
#endif

// Use 128 bit integers and supress warnings for using extensions
#if defined(BOOST_HAS_INT128)
#  define BOOST_CHARCONV_HAS_INT128
#  define BOOST_CHARCONV_INT128_MAX  (boost::int128_type)(((boost::uint128_type) 1 << 127) - 1)
#  define BOOST_CHARCONV_INT128_MIN  (-BOOST_CHARCONV_INT128_MAX - 1)
#  define BOOST_CHARCONV_UINT128_MAX ((2 * (boost::uint128_type) BOOST_CHARCONV_INT128_MAX) + 1)
#endif

#ifndef BOOST_NO_CXX14_CONSTEXPR
#  define BOOST_CHARCONV_CXX14_CONSTEXPR BOOST_CXX14_CONSTEXPR
#else
#  define BOOST_CHARCONV_CXX14_CONSTEXPR inline
#endif

#if defined(__GNUC__) && __GNUC__ == 5
#  define BOOST_CHARCONV_GCC5_CONSTEXPR inline
#else
#  define BOOST_CHARCONV_GCC5_CONSTEXPR BOOST_CHARCONV_CXX14_CONSTEXPR
#endif

// 64 bytes on x86-64 and ARM │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
#if defined(__cpp_lib_hardware_interference_size) && __cpp_lib_hardware_interference_size >= 201703L
#  define BOOST_CHARCONV_HARDWARE_DESTRUCTIVE_INTERFACE_SIZE std::hardware_destructive_interference_size
#else
#  define BOOST_CHARCONV_HARDWARE_DESTRUCTIVE_INTERFACE_SIZE 64
#endif

// Determine endianness
#if defined(_WIN32)

#define BOOST_CHARCONV_ENDIAN_BIG_BYTE 0
#define BOOST_CHARCONV_ENDIAN_LITTLE_BYTE 1

#elif defined(__BYTE_ORDER__)

#define BOOST_CHARCONV_ENDIAN_BIG_BYTE (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BOOST_CHARCONV_ENDIAN_LITTLE_BYTE (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#else

#error Could not determine endian type. Please file an issue at https://github.com/cppalliance/charconv with your architecture

#endif // Determine endianness

static_assert((BOOST_CHARCONV_ENDIAN_BIG_BYTE || BOOST_CHARCONV_ENDIAN_LITTLE_BYTE) &&
             !(BOOST_CHARCONV_ENDIAN_BIG_BYTE && BOOST_CHARCONV_ENDIAN_LITTLE_BYTE),
"Inconsistent endianness detected. Please file an issue at https://github.com/cppalliance/charconv with your architecture");

#endif // BOOST_CHARCONV_DETAIL_CONFIG_HPP
