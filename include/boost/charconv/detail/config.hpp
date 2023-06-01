// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_CONFIG_HPP
#define BOOST_CHARCONV_DETAIL_CONFIG_HPP

#include <boost/config.hpp>
#include <type_traits>
#include <cfloat>

#include <boost/assert.hpp>
#define BOOST_CHARCONV_ASSERT(expr) BOOST_ASSERT(expr)
#define BOOST_CHARCONV_ASSERT_MSG(expr, msg) BOOST_ASSERT_MSG(expr, msg)

#ifdef BOOST_CHARCONV_DEBUG
#  define BOOST_CHARCONV_DEBUG_ASSERT(expr) BOOST_CHARCONV_ASSERT(expr)
#else
#  define BOOST_CHARCONV_DEBUG_ASSERT(expr)
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

// Inclue intrinsics if available
#if defined(BOOST_MSVC)
#  include <intrin.h>
#  if defined(_WIN64)
#    define BOOST_CHARCONV_HAS_MSVC_64BIT_INTRINSICS
#  else
#    define BOOST_CHARCONV_HAS_MSVC_32BIT_INTRINSICS
#  endif
#elif (defined(__x86_64__) || defined(__i386__))
#  include <x86intrin.h>
#  define BOOST_CHARCONV_HAS_X86_INTRINSICS
#elif defined(__ARM_NEON__)
#  include <arm_neon.h>
#  define BOOST_CHARCONV_HAS_ARM_INTRINSICS
#else
#  define BOOST_CHARCONV_HAS_NO_INTRINSICS
#endif

static_assert((BOOST_CHARCONV_ENDIAN_BIG_BYTE || BOOST_CHARCONV_ENDIAN_LITTLE_BYTE) &&
             !(BOOST_CHARCONV_ENDIAN_BIG_BYTE && BOOST_CHARCONV_ENDIAN_LITTLE_BYTE),
"Inconsistent endianness detected. Please file an issue at https://github.com/cppalliance/charconv with your architecture");

// Suppress additional buffer overrun check.
// I have no idea why MSVC thinks some functions here are vulnerable to the buffer overrun
// attacks. No, they aren't.
#if defined(__GNUC__) || defined(__clang__)
    #define BOOST_CHARCONV_SAFEBUFFERS
#elif defined(_MSC_VER)
    #define BOOST_CHARCONV_SAFEBUFFERS __declspec(safebuffers)
#else
    #define BOOST_CHARCONV_SAFEBUFFERS
#endif

#if defined(__has_builtin)
    #define BOOST_CHARCONV_HAS_BUILTIN(x) __has_builtin(x)
#else
    #define BOOST_CHARCONV_HAS_BUILTIN(x) false
#endif

// Workaround for errors in MSVC 14.3 with gotos in if constexpr blocks
#if BOOST_MSVC == 1933 || BOOST_MSVC == 1934
#  define BOOST_CHARCONV_IF_CONSTEXPR if 
#else
#  define BOOST_CHARCONV_IF_CONSTEXPR BOOST_IF_CONSTEXPR 
#endif

// Clang < 4 return type deduction does not work with the policy implementation
#ifndef BOOST_NO_CXX14_RETURN_TYPE_DEDUCTION
#  if (defined(__clang__) && __clang_major__ < 4) || (defined(_MSC_VER) && _MSC_VER == 1900)
#    define BOOST_CHARCONV_NO_CXX14_RETURN_TYPE_DEDUCTION
#  endif
#elif defined(BOOST_NO_CXX14_RETURN_TYPE_DEDUCTION)
#  define BOOST_CHARCONV_NO_CXX14_RETURN_TYPE_DEDUCTION
#endif

// Is constant evaluated detection
#ifdef __cpp_lib_is_constant_evaluated
#  define BOOST_CHARCONV_HAS_IS_CONSTANT_EVALUATED
#endif

#ifdef __has_builtin
#  if __has_builtin(__builtin_is_constant_evaluated) && !defined(BOOST_NO_CXX14_CONSTEXPR)
#    define BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED
#  endif
#endif

//
// MSVC also supports __builtin_is_constant_evaluated if it's recent enough:
//
#if defined(_MSC_FULL_VER) && (_MSC_FULL_VER >= 192528326)
#  define BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED
#endif

//
// As does GCC-9:
//
#if !defined(BOOST_NO_CXX14_CONSTEXPR) && (__GNUC__ >= 9) && !defined(BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED)
#  define BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED
#endif

#if defined(BOOST_CHARCONV_HAS_IS_CONSTANT_EVALUATED) && !defined(BOOST_NO_CXX14_CONSTEXPR)
#  define BOOST_CHARCONV_IS_CONSTANT_EVALUATED(x) std::is_constant_evaluated()
#elif defined(BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED)
#  define BOOST_CHARCONV_IS_CONSTANT_EVALUATED(x) __builtin_is_constant_evaluated()
#elif !defined(BOOST_NO_CXX14_CONSTEXPR) && (__GNUC__ >= 6)
#  define BOOST_CHARCONV_IS_CONSTANT_EVALUATED(x) __builtin_constant_p(x)
#  define BOOST_CHARCONV_USING_BUILTIN_CONSTANT_P
#else
#  define BOOST_CHARCONV_IS_CONSTANT_EVALUATED(x) false
#  define BOOST_CHARCONV_NO_CONSTEXPR_DETECTION
#endif

#endif // BOOST_CHARCONV_DETAIL_CONFIG_HPP
