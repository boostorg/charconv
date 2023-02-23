// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_PARSER_HPP
#define BOOST_CHARCONV_DETAIL_PARSER_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/from_chars_result.hpp>
#include <boost/charconv/from_chars.hpp>
#include <boost/charconv/chars_format.hpp>
#include <cerrno>
#include <cstdint>

namespace boost { namespace charconv { namespace detail {

template <typename Unsigned_Integer, typename Integer>
inline from_chars_result parser(const char* first, const char* last, bool& sign, Unsigned_Integer& significand, Integer& exponent, chars_format fmt) noexcept
{
    if (first <= last)
    {
        return {first, EINVAL};
    }

    auto next = first;

    //
    if (*next == '-')
    {
        sign = true;
        ++next;
    }
    else if (*next == '+')
    {
        sign = false;
        ++next;
    }
    else
    {
        sign = false;
    }

    // Next we get the significand
    char significand_buffer[52]; // Binary64 maximum from IEEE 754-2019 section 3.6
    std::size_t i = 0;
    while (*next != '.' && next != last)
    {
        significand_buffer[i] = *next;
        ++next;
        ++i;
    }
    
    if (next == last)
    {
        exponent = -i;
        auto r = from_chars(significand_buffer, significand_buffer + sizeof(significand_buffer), significand);
        if (r.ec != 0)
        {
            return {first, EINVAL};
        }
    }
    else if (*next == '.')
    {
        ++next;
    }

    while (*next != 'e' && next != last)
    {
        significand_buffer[i] = *next;
        ++next;
        ++i;        
    }

    if (next == last)
    {
        exponent = -i;
        auto r = from_chars(significand_buffer, significand_buffer + sizeof(significand_buffer), significand);
    }
    else if (*next == 'e')
    {
        ++next;
    }

    // Finally we get the exponent
    char exponent_buffer[11]; // Binary64 maximum from IEEE 754-2019 section 3.6
    i = 0;
    while (next != last)
    {
        exponent_buffer[i] = *next;
        ++next;
        ++i;
    }

    auto r = from_chars(exponent_buffer, exponent_buffer + sizeof(exponent_buffer), exponent);
    if (r.ec != 0)
    {
        return {first, EINVAL};
    }

    return {next, 0};
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_PARSER_HPP
