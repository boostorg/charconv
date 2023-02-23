// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
#define BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <boost/charconv/to_chars.hpp>
#include <string>
#include <bitset>
#include <cstdint>
#include <cstring>
#include <cmath>

namespace boost { namespace charconv { namespace detail {

inline long double compute_float80(std::int64_t power, std::uint64_t i, bool negative, bool& success) noexcept
{
    static constexpr auto e_bias = 16383; // See IEEE 754-2019 Section 3.6 binary128

    if (power > e_bias || power < 2 - e_bias)
    {
        success = false;
        return negative ? -0.0L : 0.0L; 
    }

    char buffer[80];
    if (negative)
    {
        buffer[0] = static_cast<char>(48);
    }
    else
    {
        buffer[0] = static_cast<char>(49);
    }

    char power_buffer[15];
    to_chars(power_buffer, power_buffer + sizeof(power_buffer), e_bias - power, 2);
    std::memcpy(buffer + 1, power_buffer, sizeof(power_buffer));

    char significand_buffer[64];
    to_chars(significand_buffer, significand_buffer + sizeof(significand_buffer), i, 2);
    std::memcpy(buffer + 16, significand_buffer, sizeof(significand_buffer));


    std::bitset<80> bit_val {buffer};
    long double return_val;

    std::memcpy(&return_val, &bit_val, sizeof(return_val));

    success = true;
    return return_val;
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
