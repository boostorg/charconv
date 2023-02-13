// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED

#include <boost/charconv/detail/apply_sign.hpp>
#include <boost/charconv/config.hpp>
#include <boost/config.hpp>
#include <type_traits>
#include <limits>
#include <array>
#include <cstdlib>
#include <cerrno>
#include <cstddef>
#include <cstdint>

namespace boost { namespace charconv {

// 22.13.3, Primitive numerical input conversion

struct from_chars_result
{
    const char* ptr;

    // Values:
    // 0 = no error
    // EINVAL = invalid_argument
    // ERANGE = result_out_of_range
    int ec;

    friend constexpr bool operator==(const from_chars_result& lhs, const from_chars_result& rhs) noexcept
    {
        return lhs.ptr == rhs.ptr && lhs.ec == rhs.ec;
    }

    friend constexpr bool operator!=(const from_chars_result& lhs, const from_chars_result& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

namespace detail {

static constexpr std::array<unsigned char, 256> uchar_values =
    {{255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9, 255, 255, 255, 255, 255, 255,
      255,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
       25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35, 255, 255, 255, 255, 255,
      255,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
       25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}};

// Convert characters for 0-9, A-Z, a-z to 0-35. Anything else is 255
constexpr unsigned char digit_from_char(char val) noexcept
{
    return uchar_values[static_cast<std::size_t>(val)];
}

#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4146)
#endif

template <typename Integer, typename Unsigned_Integer>
BOOST_CXX14_CONSTEXPR from_chars_result from_chars_integer_impl(const char* first, const char* last, Integer& value, int base) noexcept
{
    Unsigned_Integer result = 0;
    Unsigned_Integer overflow_value = 0;
    Unsigned_Integer max_digit = 0;
    
    // Check pre-conditions
    BOOST_CHARCONV_ASSERT_MSG(base >= 2 && base <= 36, "Base must be between 2 and 36 (inclusive)");
    if (!(first <= last))
    {
        return {first, EINVAL};
    }

    Unsigned_Integer unsigned_base = static_cast<Unsigned_Integer>(base);

    // Strip sign if the type is signed
    // Negative sign will be appended at the end of parsing
    BOOST_ATTRIBUTE_UNUSED bool is_negative = false;
    auto next = first;

    #ifdef BOOST_CHARCONV_HAS_INT128
    BOOST_IF_CONSTEXPR (std::is_same<Integer, boost::int128_type>::value || std::is_signed<Integer>::value)
    #else
    BOOST_IF_CONSTEXPR (std::is_signed<Integer>::value)
    #endif
    {
        if (next != last)
        {
            if (*next == '-')
            {
                is_negative = true;
                ++next;
            }
            else if (*next == '+')
            {
                ++next;
            }
        }

        #ifdef BOOST_CHARCONV_HAS_INT128
        BOOST_IF_CONSTEXPR (std::is_same<Integer, boost::int128_type>::value)
        {
            overflow_value = BOOST_CHARCONV_INT128_MAX;
            max_digit = BOOST_CHARCONV_INT128_MAX;
        }
        else
        #endif
        {
            overflow_value = (std::numeric_limits<Integer>::max)();
            max_digit = (std::numeric_limits<Integer>::max)();
        }

        if (is_negative)
        {
            ++overflow_value;
            ++max_digit;
        }
    }
    else
    {
        if (next != last)
        {
            if (*next == '-')
            {
                return {first, EINVAL};
            }
            else if (*next == '+')
            {
                ++next;
            }
        }
        
        #ifdef BOOST_CHARCONV_HAS_INT128
        BOOST_IF_CONSTEXPR (std::is_same<Integer, boost::uint128_type>::value)
        {
            overflow_value = BOOST_CHARCONV_UINT128_MAX;
            max_digit = BOOST_CHARCONV_UINT128_MAX;
        }
        else
        #endif
        {
            overflow_value = (std::numeric_limits<Unsigned_Integer>::max)();
            max_digit = (std::numeric_limits<Unsigned_Integer>::max)();
        }
    }

    #ifdef BOOST_CHARCONV_HAS_INT128
    BOOST_IF_CONSTEXPR (!std::is_same<Integer, boost::int128_type>::value)
    #endif
    {
        overflow_value /= unsigned_base;
        max_digit %= unsigned_base;
    }

    // If the only character was a sign abort now
    if (next == last)
    {
        return {first, EINVAL};
    }

    bool overflowed = false;
    while (next != last)
    {
        const unsigned char current_digit = digit_from_char(*next);

        if (current_digit >= unsigned_base)
        {
            break;
        }

        if (result < overflow_value || (result == overflow_value && current_digit <= max_digit))
        {
            result = static_cast<Unsigned_Integer>(result * unsigned_base + current_digit);
        }
        else
        {
            // Required to keep updating the value of next, but the result is garbage
            overflowed = true;
        }

        ++next;
    }

    // Return the parsed value, adding the sign back if applicable
    // If we have overflowed then we do not return the result 
    if (overflowed)
    {
        return {next, ERANGE};
    }

    value = static_cast<Integer>(result);
    #ifdef BOOST_CHARCONV_HAS_INT128
    BOOST_IF_CONSTEXPR (std::is_same<Integer, boost::int128_type>::value || std::is_signed<Integer>::value)
    #else
    BOOST_IF_CONSTEXPR (std::is_signed<Integer>::value)
    #endif
    {
        if (is_negative)
        {
            value = -(static_cast<Unsigned_Integer>(value));
        }
    }

    return {next, 0};
}

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif

// Only from_chars for integer types is constexpr (as of C++23)
template <typename Integer>
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, Integer& value, int base = 10) noexcept
{
    using Unsigned_Integer = typename std::make_unsigned<Integer>::type;
    return detail::from_chars_integer_impl<Integer, Unsigned_Integer>(first, last, value, base);
}

#ifdef BOOST_HAS_INT128
template <typename Integer>
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars128(const char* first, const char* last, Integer& value, int base = 10) noexcept
{
    using Unsigned_Integer = boost::uint128_type;
    return detail::from_chars_integer_impl<Integer, Unsigned_Integer>(first, last, value, base);
}
#endif

} // Namespace detail

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

// floating point overloads

BOOST_CHARCONV_DECL from_chars_result from_chars( char const* first, char const* last, float& value ) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars( char const* first, char const* last, double& value ) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars( char const* first, char const* last, long double& value ) noexcept;

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
