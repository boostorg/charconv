// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED

#include <boost/charconv/detail/integer_search_trees.hpp>
#include <boost/charconv/config.hpp>
#include <type_traits>
#include <array>
#include <limits>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdint>

namespace boost { namespace charconv {

// 22.13.2, Primitive numerical output conversion

struct to_chars_result
{
    char* ptr;

    // Values:
    // 0 = no error
    // EINVAL = invalid_argument
    // ERANGE = result_out_of_range
    int ec;

    constexpr friend bool operator==(const to_chars_result& lhs, const to_chars_result& rhs) noexcept
    {
        return lhs.ptr == rhs.ptr && lhs.ec == rhs.ec;
    }

    constexpr friend bool operator!=(const to_chars_result& lhs, const to_chars_result& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

namespace detail {

    static constexpr char radix_table[] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4',
    '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
    '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
    '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
    '2', '0', '2', '1', '2', '2', '2', '3', '2', '4',
    '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4',
    '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
    '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
    '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4',
    '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4',
    '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
    '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
    '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
    '8', '0', '8', '1', '8', '2', '8', '3', '8', '4',
    '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4',
    '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
};

// See: https://jk-jeon.github.io/posts/2022/02/jeaiii-algorithm/
// https://arxiv.org/abs/2101.11408
BOOST_CXX14_CONSTEXPR char* decompose32(std::uint32_t value, char* buffer)
{
    constexpr auto mask = (static_cast<std::uint64_t>(1) << 57) - 1; // D = 57 so 2^D - 1
    constexpr auto magic_multiplier = static_cast<std::uint64_t>(1441151881); // floor(2*D / 10*k) where D is 57 and k is 8
    auto y = value * magic_multiplier;

    for (std::size_t i {}; i < 10; i += 2)
    {
        // Replaces std::memcpy(buffer + i, radix_table + static_cast<std::size_t>(y >> 57) * 2, 2)
        // since it would not be constexpr
        const char* temp = {radix_table + static_cast<std::size_t>(y >> 57) * 2};
        buffer[i] = temp[0];
        buffer[i+1] = temp[1];
        y &= mask;
        y *= 100;
    }

    return buffer + 10;
}

template <typename Integer>
BOOST_CXX14_CONSTEXPR to_chars_result to_chars_integer_impl(char* first, char* last, Integer value)
{   
    // decompose32 will always return 10 digits (including leading 0s)
    char buffer[10] {};
    
    if (!(first <= last))
    {
        return {last, EINVAL};
    }
    
    // If the type is less than 32 bits we can use this without change
    // If the type is greater than 32 bits we use a binary search tree to figure out how many digits
    // are present and then decompose the value into two (or more) std::uint32_t of known length so that we
    // don't have the issue of removing leading zeros from the least significant digits
    if (static_cast<std::uint32_t>(value) <= (std::numeric_limits<std::uint32_t>::max)())
    {
        decompose32(value, buffer);

        std::size_t i {};
        while (buffer[i] == '0')
        {
            ++i;
        }
        std::memcpy(first, buffer + i, sizeof(buffer) - i);
    }
    else if (static_cast<std::uint64_t>(value) <= (std::numeric_limits<std::uint64_t>::max)())
    {

    }
    #if 0
    // unsigned __128 requires 4 shifts
    // Could just recursivly call the uint64_t twice and then compose 2x 64 bits
    else if (static_cast<unsigned __int128>(value) <= (std::numeric_limits<unsigned __int128>::max)())
    #endif
    else 
    {
        BOOST_CHARCONV_ASSERT_MSG(sizeof(Integer) < 1, "Your type is unsupported. Use a built-in integral type");
    }
    
    
    return {first + std::strlen(first), 0};
}

// All other bases
template <typename Integer>
BOOST_CXX14_CONSTEXPR to_chars_result to_chars_integer_impl(char* first, char* last, Integer value, int base) noexcept
{
    BOOST_CHARCONV_ASSERT_MSG(base >= 2 && base <= 36, "Base must be between 2 and 36 (inclusive)");
    (void)base;
    if (!(first <= last))
    {
        return {last, EINVAL};
    }

    std::snprintf( first, last - first - 1, "%d", value );
    return { first + std::strlen( first ), 0 };
}

} // Namespace detail

template <typename Integer, typename std::enable_if<std::is_integral<Integer>::value, bool>::type = true>
BOOST_CXX14_CONSTEXPR to_chars_result to_chars(char* first, char* last, Integer value, int base = 10) noexcept
{
    if (base == 10)
    {
        return detail::to_chars_integer_impl(first, last, value);
    }

    return detail::to_chars_integer_impl(first, last, value, base);
}

template <>
BOOST_CXX14_CONSTEXPR to_chars_result to_chars<bool>(char* first, char* last, bool value, int base) noexcept = delete;

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
