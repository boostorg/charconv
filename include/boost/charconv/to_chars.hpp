// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED

#include <boost/charconv/detail/apply_sign.hpp>
#include <boost/charconv/detail/integer_search_trees.hpp>
#include <boost/charconv/detail/integer_conversion.hpp>
#include <boost/charconv/config.hpp>
#include <type_traits>
#include <array>
#include <limits>
#include <utility>
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

    static constexpr char digit_table[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z'
    };

// See: https://jk-jeon.github.io/posts/2022/02/jeaiii-algorithm/
// https://arxiv.org/abs/2101.11408
BOOST_CXX14_CONSTEXPR char* decompose32(std::uint32_t value, char* buffer) noexcept
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

#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4127)
#endif

// TODO: Use a temp buffer to hold everything and then write it all into the provided buffer at the end
// If the temp buffer is larger than the provided buffer return EOVERFLOW
template <typename Integer>
BOOST_CXX14_CONSTEXPR to_chars_result to_chars_integer_impl(char* first, char* last, Integer value) noexcept
{       
    char buffer[10] {};
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
            value = apply_sign(value);
            is_negative = true;
        }
    }

    // If the type is less than 32 bits we can use this without change
    // If the type is greater than 32 bits we use a binary search tree to figure out how many digits
    // are present and then decompose the value into two (or more) std::uint32_t of known length so that we
    // don't have the issue of removing leading zeros from the least significant digits
    
    // Yields: warning C4127: conditional expression is constant becuase first half of the expression is constant
    // but we need to short circuit to avoid UB on the second half
    if (std::numeric_limits<Integer>::digits <= std::numeric_limits<std::uint32_t>::digits ||
        value <= static_cast<Integer>((std::numeric_limits<std::uint32_t>::max)()))
    {
        const auto converted_value = static_cast<std::uint32_t>(value);
        const auto num_sig_chars = num_digits(converted_value);

        decompose32(converted_value, buffer);

        // TODO: If constant evaluated use unrolled loop
        // If not constant evaluated use memcpy
        if (is_negative)
        {
            *first++ = '-';
        }
            
        std::memcpy(first, buffer + (sizeof(buffer) - num_sig_chars), num_sig_chars);
    }
    else if (std::numeric_limits<Integer>::digits <= std::numeric_limits<std::uint64_t>::digits ||
             static_cast<std::uint64_t>(value) <= (std::numeric_limits<std::uint64_t>::max)())
    {
        auto converted_value = static_cast<std::uint64_t>(value);
        const auto converted_value_digits = num_digits(converted_value);

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
            std::memcpy(first, buffer + (sizeof(buffer) - first_value_chars), first_value_chars);

            decompose32(y, buffer);
            std::memcpy(first + first_value_chars, buffer + 1, sizeof(buffer) - 1);
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
                std::memcpy(first, buffer + 2, sizeof(buffer) - 2);

                decompose32(y, buffer);
                std::memcpy(first + 8, buffer + 1, sizeof(buffer) - 1);
            
                decompose32(z, buffer);
                std::memcpy(first + 17, buffer + 8, 2);
            }
            else // 20
            {
                decompose32(x, buffer);
                std::memcpy(first, buffer + 1, sizeof(buffer) - 1);

                decompose32(y, buffer);
                std::memcpy(first + 9, buffer + 1, sizeof(buffer) - 1);
            
                decompose32(z, buffer);
                std::memcpy(first + 18, buffer + 8, 2);
            }
        }
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

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif

// All other bases
// Use a simple lookup table to put together the Integer in character form
template <typename Integer>
BOOST_CXX14_CONSTEXPR to_chars_result to_chars_integer_impl(char* first, char* last, Integer value, int base) noexcept
{
    BOOST_CHARCONV_ASSERT_MSG(base >= 2 && base <= 36, "Base must be between 2 and 36 (inclusive)");

    using Unsigned_Integer = typename std::make_unsigned<Integer>::type;

    const auto output_length = last - first;

    if (!(first <= last))
    {
        return {last, EINVAL};
    }

    if (value == 0)
    {
        *first++ = '0';
        return {first, 0};
    }

    auto unsigned_value = static_cast<Unsigned_Integer>(value < 0 ? -value : value);
    const auto unsigned_base = static_cast<Unsigned_Integer>(base);

    BOOST_IF_CONSTEXPR (std::is_signed<Integer>::value)
    {
        if (value < 0)
        {
            *first++ = '-';
        }
    }

    constexpr Unsigned_Integer zero = 48U; // Char for '0'
    std::array<char, sizeof(Unsigned_Integer) * CHAR_BIT> buffer = {};
    auto end = buffer.end();
    --end; // Need to point to the last actual element

    // Work from LSB to MSB
    switch (base)
    {
    case 2:
        while (unsigned_value != 0)
        {
            *end-- = static_cast<char>(zero + (unsigned_value & 1U)); // 1<<1 - 1
            value >>= 1U;
        }
        break;

    case 4:
        while (unsigned_value != 0)
        {
            *end-- = static_cast<char>(zero + (unsigned_value & 3U)); // 1<<2 - 1
            value >>= 2U;
        }
        break;

    case 8:
        while (unsigned_value != 0)
        {
            *end-- = static_cast<char>(zero + (unsigned_value & 7U)); // 1<<3 - 1
            value >>= 3U;
        }
        break;

    case 16:
        while (unsigned_value != 0)
        {
            *end-- = static_cast<char>(zero + (unsigned_value & 15U)); // 1<<4 - 1
            value >>= 4U;
        }
        break;

    case 32:
        while (unsigned_value != 0)
        {
            *end-- = static_cast<char>(zero + (unsigned_value & 31U)); // 1<<5 - 1
            value >>= 5U;
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

    const auto num_chars = end - buffer.end() + 1;

    if (num_chars > output_length)
    {
        return {last, EOVERFLOW};
    }

    std::memcpy(first, buffer.data() + num_chars, num_chars);

    return {first + num_chars, 0};
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

// TODO: Not correct, but need to make MSVC happy while working on integers
BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, float value) noexcept;

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
