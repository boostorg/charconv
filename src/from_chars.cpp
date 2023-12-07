// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// https://stackoverflow.com/questions/38060411/visual-studio-2015-wont-suppress-error-c4996
#ifndef _SCL_SECURE_NO_WARNINGS
# define _SCL_SECURE_NO_WARNINGS
#endif
#ifndef NO_WARN_MBCS_MFC_DEPRECATION
# define NO_WARN_MBCS_MFC_DEPRECATION
#endif

#include <boost/charconv/detail/fast_float/fast_float.hpp>
#include <boost/charconv/detail/from_chars_float_impl.hpp>
#include <boost/charconv/from_chars.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <system_error>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <limits>

#if BOOST_CHARCONV_LDBL_BITS > 64
#  include <boost/charconv/detail/compute_float80.hpp>
#  include <boost/charconv/detail/emulated128.hpp>
#endif

#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, float& value, boost::charconv::chars_format fmt) noexcept
{
    if (fmt != boost::charconv::chars_format::hex)
    {
        return boost::charconv::detail::fast_float::from_chars(first, last, value, fmt);
    }
    return boost::charconv::detail::from_chars_float_impl(first, last, value, fmt);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, double& value, boost::charconv::chars_format fmt) noexcept
{
    if (fmt != boost::charconv::chars_format::hex)
    {
        return boost::charconv::detail::fast_float::from_chars(first, last, value, fmt);
    }
    return boost::charconv::detail::from_chars_float_impl(first, last, value, fmt);
}

#ifdef BOOST_CHARCONV_HAS_FLOAT128
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, __float128& value, boost::charconv::chars_format fmt) noexcept
{
    bool sign {};
    std::int64_t exponent {};

    #if defined(BOOST_CHARCONV_HAS_INT128) && ((defined(__clang_major__) && __clang_major__ > 12 ) || \
        (defined(BOOST_GCC) && BOOST_GCC > 100000))

    boost::uint128_type significand {};

    #else
    boost::charconv::detail::uint128 significand {};
    #endif

    auto r = boost::charconv::detail::parser(first, last, sign, significand, exponent, fmt);
    if (r.ec != std::errc())
    {
        return r;
    }
    else if (significand == 0)
    {
        value = sign ? -0.0Q : 0.0Q;
        return r;
    }

    std::errc success {};
    auto return_val = boost::charconv::detail::compute_float128(exponent, significand, sign, success);
    r.ec = static_cast<std::errc>(success);

    if (r.ec == std::errc() || r.ec == std::errc::result_out_of_range)
    {
        value = return_val;
    }
    else if (r.ec == std::errc::not_supported)
    {
        // Fallback routine
        errno = 0;
        std::string temp (first, last); // zero termination
        char* ptr = nullptr;
        value = strtoflt128(temp.c_str(), &ptr);
        r.ptr = ptr;
        r.ec = static_cast<std::errc>(errno);
    }

    return r;
}
#endif

#ifdef BOOST_CHARCONV_HAS_FLOAT16
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, std::float16_t& value, boost::charconv::chars_format fmt) noexcept
{
    float f;
    const auto r = boost::charconv::from_chars(first, last, f, fmt);
    if (r.ec == std::errc())
    {
        value = static_cast<std::float16_t>(f);
    }
    return r;
}
#endif

#ifdef BOOST_CHARCONV_HAS_FLOAT32
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, std::float32_t& value, boost::charconv::chars_format fmt) noexcept
{
    static_assert(std::numeric_limits<std::float32_t>::digits == FLT_MANT_DIG &&
                  std::numeric_limits<std::float32_t>::min_exponent == FLT_MIN_EXP,
                  "float and std::float32_t are not the same layout like they should be");
    
    float f;
    std::memcpy(&f, &value, sizeof(float));
    const auto r = boost::charconv::from_chars(first, last, f, fmt);
    std::memcpy(&value, &f, sizeof(std::float32_t));
    return r;
}
#endif

#ifdef BOOST_CHARCONV_HAS_FLOAT64
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, std::float64_t& value, boost::charconv::chars_format fmt) noexcept
{
    static_assert(std::numeric_limits<std::float64_t>::digits == DBL_MANT_DIG &&
                  std::numeric_limits<std::float64_t>::min_exponent == DBL_MIN_EXP,
                  "double and std::float64_t are not the same layout like they should be");
    
    double d;
    std::memcpy(&d, &value, sizeof(double));
    const auto r = boost::charconv::from_chars(first, last, d, fmt);
    std::memcpy(&value, &d, sizeof(std::float64_t));
    return r;
}
#endif

#ifdef BOOST_CHARCONV_HAS_BRAINFLOAT16
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, std::bfloat16_t& value, boost::charconv::chars_format fmt) noexcept
{
    float f;
    const auto r = boost::charconv::from_chars(first, last, f, fmt);
    if (r.ec == std::errc())
    {
        value = static_cast<std::bfloat16_t>(f);
    }
    return r;
}
#endif

#if BOOST_CHARCONV_LDBL_BITS == 64 || defined(BOOST_MSVC)

// Since long double is just a double we use the double implementation and cast into value
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long double& value, boost::charconv::chars_format fmt) noexcept
{
    static_assert(sizeof(double) == sizeof(long double), "64 bit long double detected, but the size is incorrect");
    
    double d;
    std::memcpy(&d, &value, sizeof(double));
    const auto r = boost::charconv::from_chars(first, last, d, fmt);
    std::memcpy(&value, &d, sizeof(long double));

    return r;
}

#else

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long double& value, boost::charconv::chars_format fmt) noexcept
{
    static_assert(std::numeric_limits<long double>::is_iec559, "Long double must be IEEE 754 compliant");

    bool sign {};
    std::int64_t exponent {};

    #if defined(BOOST_CHARCONV_HAS_INT128) && ((defined(__clang_major__) && __clang_major__ > 12 ) || \
        (defined(BOOST_GCC) && BOOST_GCC > 100000))

    boost::uint128_type significand {};

    #else
    boost::charconv::detail::uint128 significand {};
    #endif

    auto r = boost::charconv::detail::parser(first, last, sign, significand, exponent, fmt);
    if (r.ec != std::errc())
    {
        return r;
    }
    else if (significand == 0)
    {
        value = sign ? -0.0L : 0.0L;
        return r;
    }

    std::errc success {};
    auto return_val = boost::charconv::detail::compute_float80<long double>(exponent, significand, sign, success);
    r.ec = success;

    if (r.ec == std::errc() || r.ec == std::errc::result_out_of_range)
    {
        value = return_val;
    }
    else if (r.ec == std::errc::not_supported)
    {
        // Fallback routine
        errno = 0; // Set to zero, so we get a clean reading from strtold
        std::string temp (first, last); // zero termination
        char* ptr = nullptr;
        value = std::strtold(temp.c_str(), &ptr);
        r.ptr = ptr;

        // See: https://github.com/cppalliance/charconv/issues/103
        // The value of errno should be 0 since the conversion is successful, but it is incorrect
        if (value == 0 || value == HUGE_VALL)
        {
            r.ec = static_cast<std::errc>(errno);
        }
        else
        {
            r.ec = std::errc();
        }
    }

    return r;
}

#if defined(BOOST_CHARCONV_HAS_STDFLOAT128) && defined(BOOST_CHARCONV_HAS_FLOAT128)
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, std::float128_t& value, boost::charconv::chars_format fmt) noexcept
{
    static_assert(sizeof(__float128) == sizeof(std::float128_t));

    __float128 q;
    std::memcpy(&q, &value, sizeof(__float128));
    const auto r = boost::charconv::from_chars(first, last, q, fmt);
    std::memcpy(&value, &q, sizeof(std::float128_t));

    return r;
}
#endif

#endif // long double implementations
