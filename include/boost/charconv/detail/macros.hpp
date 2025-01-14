// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_MACROS_HPP
#define BOOST_CHARCONV_DETAIL_MACROS_HPP

// Extracted from config.hpp, with macros that should be public
// or are used in the tests
#include <boost/config.hpp>
#include <float.h>

// Use 128-bit integers and suppress warnings for using extensions
#if defined(BOOST_HAS_INT128)
#  define BOOST_CHARCONV_HAS_INT128
#  define BOOST_CHARCONV_INT128_MAX  static_cast<boost::int128_type>((static_cast<boost::uint128_type>(1) << 127) - 1)
#  define BOOST_CHARCONV_INT128_MIN  (-BOOST_CHARCONV_INT128_MAX - 1)
#  define BOOST_CHARCONV_UINT128_MAX (2 * static_cast<boost::uint128_type>(BOOST_CHARCONV_INT128_MAX) + 1)
#endif

#ifndef BOOST_NO_CXX14_CONSTEXPR
#  define BOOST_CHARCONV_CXX14_CONSTEXPR BOOST_CXX14_CONSTEXPR
#  define BOOST_CHARCONV_CXX14_CONSTEXPR_NO_INLINE BOOST_CXX14_CONSTEXPR
#else
#  define BOOST_CHARCONV_CXX14_CONSTEXPR inline
#  define BOOST_CHARCONV_CXX14_CONSTEXPR_NO_INLINE
#endif

#if defined(__GNUC__) && __GNUC__ == 5
#  define BOOST_CHARCONV_GCC5_CONSTEXPR inline
#else
#  define BOOST_CHARCONV_GCC5_CONSTEXPR BOOST_CHARCONV_CXX14_CONSTEXPR
#endif

// C++17 allowed for constexpr lambdas
#if defined(__cpp_constexpr) && __cpp_constexpr >= 201603L
#  define BOOST_CHARCONV_CXX17_CONSTEXPR constexpr
#else
#  define BOOST_CHARCONV_CXX17_CONSTEXPR inline
#endif

// Detection for C++23 fixed width floating point types
// All of these types are optional so check for each of them individually
#ifdef __STDCPP_FLOAT16_T__
#  define BOOST_CHARCONV_HAS_FLOAT16
#endif
#ifdef __STDCPP_FLOAT32_T__
#  define BOOST_CHARCONV_HAS_FLOAT32
#endif
#ifdef __STDCPP_FLOAT64_T__
#  define BOOST_CHARCONV_HAS_FLOAT64
#endif
#ifdef __STDCPP_FLOAT128_T__
#  define BOOST_CHARCONV_HAS_STDFLOAT128
#endif
#ifdef __STDCPP_BFLOAT16_T__
#  define BOOST_CHARCONV_HAS_BRAINFLOAT16
#endif

// Long double support
#if LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384 // 80 bit long double (e.g. x86-64)
#  define BOOST_CHARCONV_LDBL_BITS 80
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384 // 128 bit long double (e.g. s390x, ppcle64)
#  define BOOST_CHARCONV_LDBL_BITS 128
#elif LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024 // 64 bit long double (double == long double on ARM)
#  define BOOST_CHARCONV_LDBL_BITS 64
#else // Unsupported long double representation
#  define BOOST_CHARCONV_UNSUPPORTED_LONG_DOUBLE
#  define BOOST_CHARCONV_LDBL_BITS -1
#endif

#endif // BOOST_CHARCONV_DETAIL_CONFIG_HPP
