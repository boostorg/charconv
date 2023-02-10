// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Copyright 2023 Junekey Jeon
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED

#include <boost/charconv/detail/apply_sign.hpp>
#include <boost/charconv/detail/integer_search_trees.hpp>
#include <boost/charconv/detail/integer_conversion.hpp>
#include <boost/charconv/detail/memcpy.hpp>
#include <boost/charconv/config.hpp>
#include <type_traits>
#include <array>
#include <limits>
#include <utility>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdint>
#include <climits>

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

    static constexpr char digit_table[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z'
    };

// See: https://jk-jeon.github.io/posts/2022/02/jeaiii-algorithm/
// https://arxiv.org/abs/2101.11408
BOOST_CHARCONV_CONSTEXPR inline char* decompose32(std::uint32_t value, char* buffer) noexcept
{
    constexpr auto mask = (std::uint64_t(1) << 57) - 1;
    auto y = value * std::uint64_t(1441151881);

    for (std::size_t i {}; i < 10; i += 2)
    {
        boost::charconv::detail::memcpy(buffer + i, radix_table + static_cast<std::size_t>(y >> 57) * 2, 2);
        y &= mask;
        y *= 100;
    }

    return buffer + 10;
}

#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4127)
#endif

template <typename Integer>
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars_integer_impl(char* first, char* last, Integer value) noexcept
{
    using Unsigned_Integer = typename std::make_unsigned<Integer>::type;
    Unsigned_Integer unsigned_value {};

    char buffer[10] {};
    int converted_value_digits {};
    const std::ptrdiff_t user_buffer_size = last - first;
    BOOST_ATTRIBUTE_UNUSED bool is_negative = false;
    
    if (!(first <= last))
    {
        return {last, EINVAL};
    }

    // Strip the sign from the value and apply at the end after parsing if the type is signed
    BOOST_IF_CONSTEXPR (std::is_signed<Integer>::value)
    {
        if (value < 0)
        {
            is_negative = true;
            unsigned_value = apply_sign(value);
        }
        else
        {
            unsigned_value = value;
        }
    }
    else
    {
        unsigned_value = value;
    }

    // If the type is less than 32 bits we can use this without change
    // If the type is greater than 32 bits we use a binary search tree to figure out how many digits
    // are present and then decompose the value into two (or more) std::uint32_t of known length so that we
    // don't have the issue of removing leading zeros from the least significant digits
    
    // Yields: warning C4127: conditional expression is constant becuase first half of the expression is constant
    // but we need to short circuit to avoid UB on the second half
    if (std::numeric_limits<Integer>::digits <= std::numeric_limits<std::uint32_t>::digits ||
        unsigned_value <= static_cast<Unsigned_Integer>((std::numeric_limits<std::uint32_t>::max)()))
    {
        const auto converted_value = static_cast<std::uint32_t>(unsigned_value);
        converted_value_digits = num_digits(converted_value);

        if (converted_value_digits > user_buffer_size)
        {
            return {last, EOVERFLOW};
        }

        decompose32(converted_value, buffer);

        if (is_negative)
        {
            *first++ = '-';
        }
            
        boost::charconv::detail::memcpy(first, buffer + (sizeof(buffer) - converted_value_digits), converted_value_digits);
    }
    else if (std::numeric_limits<Integer>::digits <= std::numeric_limits<std::uint64_t>::digits ||
             static_cast<std::uint64_t>(unsigned_value) <= (std::numeric_limits<std::uint64_t>::max)())
    {
        auto converted_value = static_cast<std::uint64_t>(unsigned_value);
        converted_value_digits = num_digits(converted_value);

        if (converted_value_digits > user_buffer_size)
        {
            return {last, EOVERFLOW};
        }

        if (is_negative)
        {
            *first++ = '-';
        }

        // Only store 9 digits in each to avoid overflow
        if (num_digits(converted_value) <= 18)
        {
            const auto x = static_cast<std::uint32_t>(converted_value / UINT64_C(1000000000));
            const auto y = static_cast<std::uint32_t>(converted_value % UINT64_C(1000000000));
            const int first_value_chars = num_digits(x);

            decompose32(x, buffer);
            boost::charconv::detail::memcpy(first, buffer + (sizeof(buffer) - first_value_chars), first_value_chars);

            decompose32(y, buffer);
            boost::charconv::detail::memcpy(first + first_value_chars, buffer + 1, sizeof(buffer) - 1);
        }
        else
        {
            const auto x = static_cast<std::uint32_t>(converted_value / UINT64_C(100000000000));
            converted_value -= x * UINT64_C(100000000000);
            const auto y = static_cast<std::uint32_t>(converted_value / UINT64_C(100));
            const auto z = static_cast<std::uint32_t>(converted_value % UINT64_C(100));

            if (converted_value_digits == 19)
            {
                decompose32(x, buffer);
                boost::charconv::detail::memcpy(first, buffer + 2, sizeof(buffer) - 2);

                decompose32(y, buffer);
                boost::charconv::detail::memcpy(first + 8, buffer + 1, sizeof(buffer) - 1);
            
                decompose32(z, buffer);
                boost::charconv::detail::memcpy(first + 17, buffer + 8, 2);
            }
            else // 20
            {
                decompose32(x, buffer);
                boost::charconv::detail::memcpy(first, buffer + 1, sizeof(buffer) - 1);

                decompose32(y, buffer);
                boost::charconv::detail::memcpy(first + 9, buffer + 1, sizeof(buffer) - 1);
            
                decompose32(z, buffer);
                boost::charconv::detail::memcpy(first + 18, buffer + 8, 2);
            }
        }
    }
    else 
    {
        BOOST_CHARCONV_ASSERT_MSG(sizeof(Integer) < 1, "Your type is unsupported. Use a built-in integral type");
    }
    
    return {first + converted_value_digits, 0};
}

#ifdef BOOST_CHARCONV_HAS_INT128
// Prior to GCC 10.3 std::numeric_limits was not specialized for __int128 which breaks the above control flow
// Here we find if the 128-bit type will fit into a 64-bit type and use the above, or we use string manipulation
// to extract the digits
//
// See: https://quuxplusone.github.io/blog/2019/02/28/is-int128-integral/
template <typename Integer>
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars_128integer_impl(char* first, char* last, Integer value) noexcept
{
    using Unsigned_Integer = boost::uint128_type;
    Unsigned_Integer unsigned_value {};

    const std::ptrdiff_t user_buffer_size = last - first;
    BOOST_ATTRIBUTE_UNUSED bool is_negative = false;
    
    if (!(first <= last))
    {
        return {last, EINVAL};
    }

    // Strip the sign from the value and apply at the end after parsing if the type is signed
    BOOST_IF_CONSTEXPR (std::is_signed<Integer>::value)
    {
        if (value < 0)
        {
            is_negative = true;
            unsigned_value = apply_sign(value);
        }
        else
        {
            unsigned_value = value;
        }
    }
    else
    {
        unsigned_value = value;
    }

    auto converted_value = static_cast<boost::uint128_type>(unsigned_value);

    // If the value fits into 64 bits use the other method of processing
    if (converted_value < (std::numeric_limits<std::uint64_t>::max)())
    {
        return to_chars_integer_impl(first, last, value);
    }

    const int converted_value_digits = num_digits(converted_value);

    if (converted_value_digits > user_buffer_size)
    {
        return {last, EOVERFLOW};
    }

    if (is_negative)
    {
        *first++ = '-';
    }

    constexpr std::uint32_t ten_9 = UINT32_C(1000000000);
    char buffer[5][10] {};
    int num_chars[5] {};
    int i = 0;

    while (converted_value != 0)
    {
        auto digits = static_cast<std::uint32_t>(converted_value % ten_9);
        num_chars[i] = num_digits(digits);
        decompose32(digits, buffer[i]); // Always returns 10 digits (to include leading 0s) which we want
        converted_value = (converted_value - digits) / ten_9;
        ++i;
    }

    --i;
    boost::charconv::detail::memcpy(first, buffer[i] + 10 - num_chars[i], num_chars[i]);
    std::size_t offset = num_chars[i];

    while (i > 0)
    {
        --i;
        boost::charconv::detail::memcpy(first + offset, buffer[i] + 1, 9);
        offset += 9;
    }

    return {first + converted_value_digits, 0};
}
#endif

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif

// All other bases
// Use a simple lookup table to put together the Integer in character form
template <typename Integer>
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars_integer_impl(char* first, char* last, Integer value, int base) noexcept
{
    BOOST_CHARCONV_ASSERT_MSG(base >= 2 && base <= 36, "Base must be between 2 and 36 (inclusive)");

    using Unsigned_Integer = typename std::make_unsigned<Integer>::type;

    const std::ptrdiff_t output_length = last - first;

    if (!(first <= last))
    {
        return {last, EINVAL};
    }

    if (value == 0)
    {
        *first++ = '0';
        return {first, 0};
    }

    Unsigned_Integer unsigned_value {};    
    const auto unsigned_base = static_cast<Unsigned_Integer>(base);

    BOOST_IF_CONSTEXPR (std::is_signed<Integer>::value)
    {
        if (value < 0)
        {
            *first++ = '-';
            unsigned_value = apply_sign(value);
        }
        else
        {
            unsigned_value = value;
        }
    }
    else
    {
        unsigned_value = value;
    }

    constexpr Unsigned_Integer zero = 48U; // Char for '0'
    constexpr auto buffer_size = sizeof(Unsigned_Integer) * CHAR_BIT;
    char buffer[buffer_size] {};
    const char* buffer_end = buffer + buffer_size;
    char* end = buffer + buffer_size - 1;

    // Work from LSB to MSB
    switch (base)
    {
    case 2:
        while (unsigned_value != 0)
        {
            *end-- = static_cast<char>(zero + (unsigned_value & 1U)); // 1<<1 - 1
            unsigned_value >>= 1U;
        }
        break;

    case 4:
        while (unsigned_value != 0)
        {
            *end-- = static_cast<char>(zero + (unsigned_value & 3U)); // 1<<2 - 1
            unsigned_value >>= 2U;
        }
        break;

    case 8:
        while (unsigned_value != 0)
        {
            *end-- = static_cast<char>(zero + (unsigned_value & 7U)); // 1<<3 - 1
            unsigned_value >>= 3U;
        }
        break;

    case 16:
        while (unsigned_value != 0)
        {
            *end-- = digit_table[unsigned_value & 15U]; // 1<<4 - 1
            unsigned_value >>= 4U;
        }
        break;

    case 32:
        while (unsigned_value != 0)
        {
            *end-- = digit_table[unsigned_value & 31U]; // 1<<5 - 1
            unsigned_value >>= 5U;
        }
        break;

    default:
        while (unsigned_value != 0)
        {
            *end-- = digit_table[unsigned_value % unsigned_base];
            unsigned_value /= unsigned_base;
        }
        break;
    }

    const std::ptrdiff_t num_chars = buffer_end - end - 1;
    
    if (num_chars > output_length)
    {
        return {last, EOVERFLOW};
    }

    boost::charconv::detail::memcpy(first, buffer + (buffer_size - num_chars), num_chars);

    return {first + num_chars, 0};
}

} // Namespace detail

template <typename Integer, typename std::enable_if<std::is_integral<Integer>::value, bool>::type = true>
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, Integer value, int base = 10) noexcept
{
    if (base == 10)
    {
        #ifdef BOOST_CHARCONV_HAS_INT128
        BOOST_IF_CONSTEXPR(std::is_same<Integer, boost::int128_type>::value || std::is_same<Integer, boost::uint128_type>::value)
        {
            return detail::to_chars_128integer_impl(first, last, value);
        }
        else
        #endif
        {
            return detail::to_chars_integer_impl(first, last, value);
        }
    }

    return detail::to_chars_integer_impl(first, last, value, base);
}

template <>
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars<bool>(char* first, char* last, bool value, int base) noexcept = delete;

// floating point overloads

BOOST_CHARCONV_DECL to_chars_result to_chars( char* first, char* last, float value ) noexcept;
BOOST_CHARCONV_DECL to_chars_result to_chars( char* first, char* last, double value ) noexcept;
BOOST_CHARCONV_DECL to_chars_result to_chars( char* first, char* last, long double value ) noexcept;

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
