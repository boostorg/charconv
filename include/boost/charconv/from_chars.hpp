// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/from_chars_result.hpp>
#include <boost/charconv/detail/from_chars_integer_impl.hpp>
#include <boost/charconv/detail/parser.hpp>
#include <boost/charconv/detail/compute_float32.hpp>
#include <boost/charconv/detail/compute_float64.hpp>
#include <boost/charconv/config.hpp>
#include <boost/charconv/chars_format.hpp>
#include <cmath>

namespace boost { namespace charconv {

// integer overloads

BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, bool& value, int base = 10) noexcept = delete;
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, char& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, signed char& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned char& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, short& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned short& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, int& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned int& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, long long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned long long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}

#ifdef BOOST_CHARCONV_HAS_INT128
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, boost::int128_type& value, int base = 10) noexcept
{
    return detail::from_chars_integer_impl<boost::int128_type, boost::uint128_type>(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, boost::uint128_type& value, int base = 10) noexcept
{
    return detail::from_chars_integer_impl<boost::uint128_type, boost::uint128_type>(first, last, value, base);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
// Floating Point
//----------------------------------------------------------------------------------------------------------------------

#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4244) // Implict converion when BOOST_IF_CONSTEXPR expands to if
#elif defined(__GNUC__) && __GNUC__ < 5 && !defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace detail {

template <typename T>
from_chars_result from_chars_strtod(const char* first, const char* last, T& value) noexcept
{
    // For strto(f/d)
    // Floating point value corresponding to the contents of str on success.
    // If the converted value falls out of range of corresponding return type, range error occurs and HUGE_VAL, HUGE_VALF or HUGE_VALL is returned.
    // If no conversion can be performed, 0 is returned and *str_end is set to str.

    value = 0;
    char* str_end;
    T return_value {};
    BOOST_IF_CONSTEXPR (std::is_same<T, float>::value)
    {
        return_value = std::strtof(first, &str_end);
        if (return_value == HUGE_VALF)
        {
            return {last, ERANGE};
        }
    }
    else
    {
        return_value = std::strtod(first, &str_end);
        if (return_value == HUGE_VAL)
        {
            return {last, ERANGE};
        }
    }

    // Since this is a fallback routine we are safe to check for 0
    if (return_value == 0 && str_end == last)
    {
        return {first, EINVAL};
    }

    value = return_value;
    return {str_end, 0};
}

template <typename T>
from_chars_result from_chars_float_impl(const char* first, const char* last, T& value, chars_format fmt) noexcept
{
    bool sign {};
    std::uint64_t significand {};
    std::int64_t  exponent {};

    auto r = boost::charconv::detail::parser(first, last, sign, significand, exponent, fmt);
    if (r.ec != 0)
    {
        value = 0;
        return r;
    }

    bool success {};
    T return_val {};
    BOOST_IF_CONSTEXPR (std::is_same<T, float>::value)
    {
        return_val = compute_float32(exponent, significand, sign, success);
    }
    else
    {
        return_val = compute_float64(exponent, significand, sign, success);
    }

    if (!success)
    {
        if (significand == 1 && exponent == 0)
        {
            value = 1;
            r.ptr = last;
            r.ec = 0;
        }
        else
        {
            // Fallback to strtod to try again
            r = from_chars_strtod(first, last, value);
        }
    }
    else
    {
        value = return_val;
    }

    return r;
}

} // Namespace detail

#ifdef BOOST_MSVC
# pragma warning(pop)
#elif defined(__GNUC__) && __GNUC__ < 5 && !defined(__clang__)
# pragma GCC diagnostic pop
#endif

BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, float& value, chars_format fmt = chars_format::general) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, double& value, chars_format fmt = chars_format::general) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, long double& value, chars_format fmt = chars_format::general) noexcept;

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
