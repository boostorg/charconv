// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_PARSER_HPP
#define BOOST_CHARCONV_DETAIL_PARSER_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/from_chars_result.hpp>
#include <boost/charconv/detail/from_chars_integer_impl.hpp>
#include <boost/charconv/detail/integer_search_trees.hpp>
#include <boost/charconv/limits.hpp>
#include <boost/charconv/chars_format.hpp>
#include <type_traits>
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

    // Ignore leading zeros (e.g. 00005 or -002.3e+5)
    while (*next == '0' && next != last)
    {
        ++next;
    }

    // Next we get the significand
    constexpr std::size_t significand_buffer_size = limits<Unsigned_Integer>::max_chars10; // Base 10 or 16
    char significand_buffer[significand_buffer_size] {};
    std::size_t i = 0;
    std::size_t dot_position = 0;
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

    while (*next != '.' && *next != exp_char && *next != capital_exp_char && next != last && i < significand_buffer_size)
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
        dot_position = i;
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

        exponent = static_cast<Integer>(dot_position) - i;
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
        bool round = false;
        // If more digits are present than representable in the significand of the target type
        // we set the maximum
        BOOST_IF_CONSTEXPR(std::is_same<Unsigned_Integer, std::uint64_t>::value)
        {
            if (offset > 19)
            {
                offset = 19;
                i = 19;
                if (significand_buffer[offset] == '5' ||
                    significand_buffer[offset] == '6' ||
                    significand_buffer[offset] == '7' ||
                    significand_buffer[offset] == '8' ||
                    significand_buffer[offset] == '9')
                {
                    round = true;
                }
            }
        }
        else
        {
            if (offset > 39)
            {
                offset = 39;
                i = 39;
                if (significand_buffer[offset] == '5' ||
                    significand_buffer[offset] == '6' ||
                    significand_buffer[offset] == '7' ||
                    significand_buffer[offset] == '8' ||
                    significand_buffer[offset] == '9')
                {
                    round = true;
                }
            }
        }

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

        if (round)
        {
            significand += 1;
        }
    }

    // Finally we get the exponent
    constexpr std::size_t exponent_buffer_size = 6; // Float128 min exp is −16382
    char exponent_buffer[exponent_buffer_size] {}; // Binary64 maximum from IEEE 754-2019 section 3.6
    Integer significand_digits = i;
    i = 0;
    while (next != last && i < exponent_buffer_size)
    {
        exponent_buffer[i] = *next;
        ++next;
        ++i;
    }

    if (next != last && i == exponent_buffer_size)
    {
        return {next, ERANGE};
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
                    exponent -= num_digits(significand) - dot_position;
                }
                else
                {
                    exponent -= significand_digits - dot_position;
                }
            }
            return {next, 0};
    }
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_PARSER_HPP
