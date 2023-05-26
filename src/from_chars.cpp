// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/detail/fast_float/fast_float.hpp>
#include <boost/charconv/detail/from_chars_float_impl.hpp>
#include <boost/charconv/from_chars.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <system_error>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

std::errc boost::charconv::detail::errno_to_errc(int errno_value) noexcept
{
    switch (errno_value)
    {
        case EINVAL:
            return std::errc::invalid_argument;
        case ERANGE:
            return std::errc::result_out_of_range;
        default:
            return std::errc();
    }
}

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

/*
#if BOOST_CHARCONV_LDBL_BITS == 64 || defined(_WIN64) || defined(_WIN32)
// Since long double is just a double we use the double implementation and cast into value
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long double& value, boost::charconv::chars_format fmt) noexcept
{
    double d;
    auto r = boost::charconv::from_chars(first, last, d, fmt);
    value = static_cast<long double>(d);
    return r;
}

#elif BOOST_CHARCONV_LDBL_BITS == 80
// https://en.wikipedia.org/wiki/Extended_precision#x86_extended_precision_format
// 63 bit significand so we are still safe to use uint64_t to represent
boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long double& value, boost::charconv::chars_format fmt) noexcept
{
    bool sign {};
    std::uint64_t significand {};
    std::int64_t  exponent {};

    auto r = boost::charconv::detail::parser(first, last, sign, significand, exponent, fmt);
    if (r.ec != std::errc())
    {
        value = 0.0L;
        return r;
    }

    bool success {};
    auto return_val = boost::charconv::detail::compute_float80(exponent, significand, sign, success);
    if (!success)
    {
        value = 0.0L;
        r.ec = std::errc::result_out_of_range;
    }
    else
    {
        value = return_val;
    }

    return r;
}
#else

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
*/

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
    (void)fmt;
    from_chars_result r = {};

    std::string tmp( first, last ); // zero termination
    char* ptr = 0;

    value = std::strtold( tmp.c_str(), &ptr );

    r.ptr = ptr;
    r.ec = detail::errno_to_errc(errno);

    return r;
}

#endif // long double implementations
