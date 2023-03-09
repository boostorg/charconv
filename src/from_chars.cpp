// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/parser.hpp>
#include <boost/charconv/detail/compute_float32.hpp>
#include <boost/charconv/detail/compute_float64.hpp>
#include <boost/charconv/detail/compute_float80.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
// TODO: compute_float128.hpp
#include <boost/charconv/from_chars.hpp>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <cerrno>

#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, float& value, boost::charconv::chars_format fmt) noexcept
{
    bool sign {};
    std::uint64_t significand {};
    std::int64_t  exponent {};

    auto r = boost::charconv::detail::parser(first, last, sign, significand, exponent, fmt);
    if (r.ec != 0)
    {
        value = 0.0;
        return r;
    }

    bool success {};
    auto return_val = boost::charconv::detail::compute_float32(exponent, significand, sign, success);
    if (!success)
    {
        value = 0.0F;
        r.ec = ERANGE;
    }
    else
    {
        value = return_val;
    }

    return r;
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, double& value, boost::charconv::chars_format fmt) noexcept
{
    bool sign {};
    std::uint64_t significand {};
    std::int64_t  exponent {};

    auto r = boost::charconv::detail::parser(first, last, sign, significand, exponent, fmt);
    if (r.ec != 0)
    {
        value = 0.0;
        return r;
    }

    bool success {};
    auto return_val = boost::charconv::detail::compute_float64(exponent, significand, sign, success);
    if (!success)
    {
        value = 0.0;
        r.ec = ERANGE;
    }
    else
    {
        value = return_val;
    }

    return r;
}

#if BOOST_CHARCONV_LDBL_BITS == 64 || defined(BOOST_MSVC)
// Since long double is just a double we use the double implementation and cast into value
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long double& value, boost::charconv::chars_format fmt) noexcept
{
    double d;
    auto r = boost::charconv::from_chars(first, last, d, fmt);
    value = static_cast<long double>(d);
    return r;
}

#elif BOOST_CHARCONV_LDBL_BITS == 80 || BOOST_CHARCONV_LDBL_BITS == 128
// Works for both 80 and 128 bit long doubles becuase they both allow for normal standard library functions
// https://en.wikipedia.org/wiki/Extended_precision#x86_extended_precision_format
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long double& value, boost::charconv::chars_format fmt) noexcept
{
    bool sign {};
    boost::uint128_type significand {};
    std::int64_t exponent {};

    auto r = boost::charconv::detail::parser(first, last, sign, significand, exponent, fmt);
    if (r.ec != 0)
    {
        value = 0.0L;
        return r;
    }

    bool success {};
    long double return_val;
    if (exponent >= 4892 || exponent <= -4932)
    {
        std::string tmp(first, last);
        if (fmt == boost::charconv::chars_format::hex)
        {
            tmp.insert(0, "0x");
        }

        char* ptr = 0;
        return_val = std::strtold(tmp.c_str(), &ptr);
        r.ec = errno;
        r.ptr = ptr;
        if (r.ec == 0)
        {
            success = true;
        }
    }
    else
    {
        return_val = boost::charconv::detail::compute_float80(exponent, significand, sign, success);
    }

    if (!success)
    {
        value = 0.0L;
        r.ec = ERANGE;
    }
    else
    {
        value = return_val;
    }

    return r;
}

#else

// __float128 will need to use functions out of libquadmath
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long double& value, boost::charconv::chars_format fmt) noexcept
{
    (void)fmt;
    from_chars_result r = {};

    std::string tmp( first, last ); // zero termination
    char* ptr = 0;

    value = std::strtold( tmp.c_str(), &ptr );

    r.ptr = ptr;
    r.ec = errno;

    return r;
}

#endif // long double implementations
