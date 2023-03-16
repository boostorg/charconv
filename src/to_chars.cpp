// Copyright 2020-2023 Junekey Jeon
// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/to_chars.hpp>
#include <limits>
#include <cstdio>
#include <cstring>

namespace boost { namespace charconv { namespace detail {

// These "//"'s are to prevent clang-format to ruin this nice alignment.
// Thanks to reddit user u/mcmcc:
// https://www.reddit.com/r/cpp/comments/so3wx9/dragonbox_110_is_released_a_fast_floattostring/hw8z26r/?context=3

static constexpr char radix_100_table[] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', //
    '0', '5', '0', '6', '0', '7', '0', '8', '0', '9', //
    '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', //
    '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', //
    '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', //
    '2', '5', '2', '6', '2', '7', '2', '8', '2', '9', //
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', //
    '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', //
    '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', //
    '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', //
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', //
    '5', '5', '5', '6', '5', '7', '5', '8', '5', '9', //
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', //
    '6', '5', '6', '6', '6', '7', '6', '8', '6', '9', //
    '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', //
    '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', //
    '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', //
    '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', //
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', //
    '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'  //
};

static constexpr char radix_100_head_table[] = {
    '0', '.', '1', '.', '2', '.', '3', '.', '4', '.', //
    '5', '.', '6', '.', '7', '.', '8', '.', '9', '.', //
    '1', '.', '1', '.', '1', '.', '1', '.', '1', '.', //
    '1', '.', '1', '.', '1', '.', '1', '.', '1', '.', //
    '2', '.', '2', '.', '2', '.', '2', '.', '2', '.', //
    '2', '.', '2', '.', '2', '.', '2', '.', '2', '.', //
    '3', '.', '3', '.', '3', '.', '3', '.', '3', '.', //
    '3', '.', '3', '.', '3', '.', '3', '.', '3', '.', //
    '4', '.', '4', '.', '4', '.', '4', '.', '4', '.', //
    '4', '.', '4', '.', '4', '.', '4', '.', '4', '.', //
    '5', '.', '5', '.', '5', '.', '5', '.', '5', '.', //
    '5', '.', '5', '.', '5', '.', '5', '.', '5', '.', //
    '6', '.', '6', '.', '6', '.', '6', '.', '6', '.', //
    '6', '.', '6', '.', '6', '.', '6', '.', '6', '.', //
    '7', '.', '7', '.', '7', '.', '7', '.', '7', '.', //
    '7', '.', '7', '.', '7', '.', '7', '.', '7', '.', //
    '8', '.', '8', '.', '8', '.', '8', '.', '8', '.', //
    '8', '.', '8', '.', '8', '.', '8', '.', '8', '.', //
    '9', '.', '9', '.', '9', '.', '9', '.', '9', '.', //
    '9', '.', '9', '.', '9', '.', '9', '.', '9', '.'  //
};

}}} // Namespaces

boost::charconv::to_chars_result boost::charconv::to_chars( char* first, char* last, float value ) noexcept
{
    std::snprintf( first, last - first, "%.*g", std::numeric_limits<float>::max_digits10, value );
    return { first + std::strlen(first), 0 };
}

boost::charconv::to_chars_result boost::charconv::to_chars( char* first, char* last, double value ) noexcept
{
    std::snprintf( first, last - first, "%.*g", std::numeric_limits<double>::max_digits10, value );
    return { first + std::strlen(first), 0 };
}

boost::charconv::to_chars_result boost::charconv::to_chars( char* first, char* last, long double value ) noexcept
{
    std::snprintf( first, last - first, "%.*Lg", std::numeric_limits<long double>::max_digits10, value );
    return { first + std::strlen(first), 0 };
}
