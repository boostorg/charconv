// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/parser.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <boost/charconv/detail/compute_float32.hpp>
#include <boost/charconv/detail/compute_float64.hpp>

#if !((BOOST_CHARCONV_LDBL_BITS == 64 || defined(BOOST_MSVC))) && defined(BOOST_CHARCONV_HAS_INT128)
#include <boost/charconv/detail/compute_float80.hpp>
#endif

#include <boost/charconv/from_chars.hpp>
#include <string>
#include <cstdlib>

#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, float& value, boost::charconv::chars_format fmt) noexcept
{
    return boost::charconv::detail::from_chars_float_impl(first, last, value, fmt);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, double& value, boost::charconv::chars_format fmt) noexcept
{
    return boost::charconv::detail::from_chars_float_impl(first, last, value, fmt);
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

#elif (BOOST_CHARCONV_LDBL_BITS == 80 || BOOST_CHARCONV_LDBL_BITS == 128) && defined(BOOST_CHARCONV_HAS_INT128)
// Works for both 80 and 128 bit long doubles because they both allow for normal standard library functions
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
        char* ptr = nullptr;
        if (fmt == boost::charconv::chars_format::hex)
        {
            std::string tmp(first, last);
            tmp.insert(0, "0x");
            return_val = std::strtold(tmp.c_str(), &ptr);
        }
        else
        {
            return_val = std::strtold(first, &ptr);
        }

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

#endif // long double implementations

#ifdef BOOST_HAS_FLOAT128
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, __float128& value, boost::charconv::chars_format fmt) noexcept
{
    __float128 return_val;
    char* ptr = nullptr;
    from_chars_result r;
    if (fmt == boost::charconv::chars_format::hex)
    {
        std::string tmp(first, last);
        tmp.insert(0, "0x");
        return_val = strtoflt128(tmp.c_str(), &ptr);
    }
    else
    {
        return_val = strtoflt128(first, &ptr);
    }

    r.ec = errno;
    r.ptr = ptr;

    if (r.ec == 0)
    {
        value = return_val;
    }

    return r;
}
#endif
