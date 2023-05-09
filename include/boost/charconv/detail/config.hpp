// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_CONFIG_HPP
#define BOOST_CHARCONV_DETAIL_CONFIG_HPP

#include <boost/config.hpp>

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
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60846
// A 32 bit target on a 64 bit system will have int128 defined but unusable
#if defined(BOOST_HAS_INT128) && !defined(__i386__)
#  define BOOST_CHARCONV_HAS_INT128
#  define BOOST_CHARCONV_INT128_MAX  (boost::int128_type)(((boost::uint128_type) 1 << 127) - 1)
#  define BOOST_CHARCONV_INT128_MIN  (-BOOST_CHARCONV_INT128_MAX - 1)
#  define BOOST_CHARCONV_UINT128_MAX ((2 * (boost::uint128_type) BOOST_CHARCONV_INT128_MAX) + 1)
#endif

// 128 bit floats
#if defined(BOOST_HAS_FLOAT128) 
#  define BOOST_CHARCONV_HAS_FLOAT128
#  include <quadmath.h>
namespace boost { namespace charconv {
    using float128_type = __float128;
}}
#elif defined(BOOST_INTEL)
#  define BOOST_CHARCONV_HAS_FLOAT128
namespace boost { namespace charconv {
    using float128_type = _Quad;
    extern "C"
    {
        _Quad __powq(_Quad, _Quad);
        #define powq __powq
    }
}}
#else
#  define BOOST_CHARCONV_NO_FLOAT128
#endif

// If we have either type of float128_t we can define the numerical limits
#ifndef BOOST_CHARCONV_NO_FLOAT128
#include <cfloat>
namespace boost { namespace charconv { namespace quad_constants {
static constexpr float128_type quad_min = static_cast<float128_type>(1) * static_cast<float128_type>(DBL_MIN) * 
                                          static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                          static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                          static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                          static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                          static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                          static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                          static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                          static_cast<float128_type>(DBL_MIN) / 1073741824;

static constexpr float128_type quad_denorm_min = static_cast<float128_type>(1) * static_cast<float128_type>(DBL_MIN) * 
                                                 static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                                 static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                                 static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                                 static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                                 static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                                 static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                                 static_cast<float128_type>(DBL_MIN) * static_cast<float128_type>(DBL_MIN) * 
                                                 static_cast<float128_type>(DBL_MIN) / 5.5751862996326557854e+42L;

static constexpr double dbl_mult = 8.9884656743115795386e+307; // This has one bit set only.
static constexpr float128_type quad_max = (static_cast<float128_type>(1) - 9.62964972193617926527988971292463659e-35) // This now has all bits sets to 1
                                          * static_cast<float128_type>(dbl_mult) * static_cast<float128_type>(dbl_mult) * 
                                          static_cast<float128_type>(dbl_mult) * static_cast<float128_type>(dbl_mult) * 
                                          static_cast<float128_type>(dbl_mult) * static_cast<float128_type>(dbl_mult) * 
                                          static_cast<float128_type>(dbl_mult) * static_cast<float128_type>(dbl_mult) * 
                                          static_cast<float128_type>(dbl_mult) * static_cast<float128_type>(dbl_mult) * 
                                          static_cast<float128_type>(dbl_mult) * static_cast<float128_type>(dbl_mult) * 
                                          static_cast<float128_type>(dbl_mult) * static_cast<float128_type>(dbl_mult) * 
                                          static_cast<float128_type>(dbl_mult) * static_cast<float128_type>(dbl_mult) * 65536;
}}} // namespace quad_constants

#  define BOOST_CHARCONV_QUAD_MIN boost::charconv::quad_constants::quad_min
#  define BOOST_CHARCONV_QUAD_DENORM_MIN boost::charconv::quad_constants::quad_denorm_min
#  define BOOST_CHARCONV_QUAD_MAX boost::charconv::quad_constants::quad_max
#endif // Quad constants

#if defined(BOOST_CHARCONV_LIBQUADMATH_FLOAT128) && defined(BOOST_CHARCONV_INTEL_FLOAT128)
#  error "Inconsistent float128 implementations detected"
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

#endif // BOOST_CHARCONV_DETAIL_CONFIG_HPP
