// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_PARSER_HPP
#define BOOST_CHARCONV_DETAIL_PARSER_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/from_chars_result.hpp>
#include <boost/charconv/detail/from_chars_integer_impl.hpp>
#include <boost/charconv/detail/integer_search_trees.hpp>
#include <boost/charconv/chars_format.hpp>
#include <limits>
#include <cerrno>
#include <cstdint>
#include <cstring>

namespace boost { namespace charconv { namespace detail {

template <typename Unsigned_Integer, typename Integer>
inline from_chars_result parser(const char* first, const char* last, bool& sign, Unsigned_Integer& significand, Integer& exponent, chars_format fmt = chars_format::general) noexcept
{
    if (!(first <= last))
    {
        return {first, EINVAL};
    }

    auto next = first;

    // First extract the sign
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
    char significand_buffer[52] {}; // Binary64 maximum from IEEE 754-2019 section 3.6
    std::size_t i = 0;
    char exp_char;
    char capital_exp_char;
    if (fmt != chars_format::hex)
    {
        exp_char = 'e';
        capital_exp_char = 'E';
    }
    else
    {
        exp_char = 'p';
        capital_exp_char = 'P';
    }

    while (*next != '.' && *next != exp_char && *next != capital_exp_char && next != last)
    {
        significand_buffer[i] = *next;
        ++next;
        ++i;
    }

    bool fractional = false;
    if (next == last)
    {
        // if fmt is chars_format::scientific the e is required
        if (fmt == chars_format::scientific)
        {
            return {first, EINVAL};
        }
        
        exponent = 0;
        std::size_t offset = i;

        from_chars_result r;
        if (fmt == chars_format::hex)
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand, 16);
        }
        else
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand);
        }
        switch (r.ec)
        {
            case EINVAL:
                return {first, EINVAL};
            case ERANGE:
                return {next, ERANGE};
            default:
                return {next, 0};
        }
    }
    else if (*next == '.')
    {
        ++next;
        fractional = true;
    }

    // if fmt is chars_format::scientific the e is required
    // if fmt is chars_format::fixed and not scientific the e is disallowed
    // if fmt is chars_format::general (which is scientific and fixed) the e is optional
    while (*next != exp_char && *next != capital_exp_char && next != last)
    {
        significand_buffer[i] = *next;
        ++next;
        ++i;        
    }

    if (next == last)
    {
        if (fmt == chars_format::scientific)
        {
            return {first, EINVAL};
        }
        
        exponent = i - 1;
        std::size_t offset = i;
        
        from_chars_result r;
        if (fmt == chars_format::hex)
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand, 16);
        }
        else
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand);
        }
        switch (r.ec)
        {
            case EINVAL:
                return {first, EINVAL};
            case ERANGE:
                return {next, ERANGE};
            default:
                return {next, 0};
        }
    }
    else if (*next == exp_char || *next == capital_exp_char)
    {
        ++next;
        if (fmt == chars_format::fixed)
        {
            return {first, EINVAL};
        }

        exponent = i - 1;
        std::size_t offset = i;

        from_chars_result r;
        if (fmt == chars_format::hex)
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand, 16);
        }
        else
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand);
        }
        switch (r.ec)
        {
            case EINVAL:
                return {first, EINVAL};
            case ERANGE:
                return {next, ERANGE};
        }
    }

    // Finally we get the exponent
    char exponent_buffer[11] {}; // Binary64 maximum from IEEE 754-2019 section 3.6
    Integer significand_digits = i;
    i = 0;
    while (next != last)
    {
        exponent_buffer[i] = *next;
        ++next;
        ++i;
    }

    auto r = from_chars(exponent_buffer, exponent_buffer + std::strlen(exponent_buffer), exponent);
    switch (r.ec)
    {
        case EINVAL:
            return {first, EINVAL};
        case ERANGE:
            return {next, ERANGE};
        default:
            if (fractional)
            {
                // Need to take the offset from 1.xxx becuase compute_floatXXX assumes the significand is an integer
                // so the exponent is off by the number of digits in the significand - 1
                if (fmt == chars_format::hex)
                {
                    // In hex the number of digits parsed is possibly less than the number of digits in base10
                    exponent -= num_digits(significand) - 1;
                }
                else
                {
                    exponent -= significand_digits - 1;
                }
            }
            return {next, 0};
    }
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_PARSER_HPP
