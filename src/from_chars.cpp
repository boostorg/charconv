// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/parser.hpp>
#include <boost/charconv/detail/compute_float32.hpp>
#include <boost/charconv/detail/compute_float64.hpp>
#include <boost/charconv/detail/compute_float80.hpp>
// TODO: compute_float128.hpp
#include <boost/charconv/from_chars.hpp>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <cerrno>

#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

boost::charconv::from_chars_result boost::charconv::from_chars(char const* first, char const* last, float& value, boost::charconv::chars_format fmt) noexcept
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
        r.ec = EINVAL;
    }
    else
    {
        value = return_val;
    }

    return r;
}

boost::charconv::from_chars_result boost::charconv::from_chars(char const* first, char const* last, double& value, boost::charconv::chars_format fmt) noexcept
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
        r.ec = EINVAL;
    }
    else
    {
        value = return_val;
    }

    return r;
}

boost::charconv::from_chars_result boost::charconv::from_chars(char const* first, char const* last, long double& value, boost::charconv::chars_format fmt) noexcept
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
