// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Copyright 2023 Junekey Jeon
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED

#include <boost/charconv/detail/apply_sign.hpp>
#include <boost/charconv/detail/integer_search_trees.hpp>
#include <boost/charconv/detail/memcpy.hpp>
#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/dragonbox/floff.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <boost/charconv/detail/dragonbox/dragonbox.hpp>
#include <boost/charconv/detail/to_chars_integer_impl.hpp>
#include <boost/charconv/detail/to_chars_result.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <boost/charconv/config.hpp>
#include <boost/charconv/chars_format.hpp>
#include <system_error>
#include <type_traits>
#include <array>
#include <limits>
#include <utility>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdint>
#include <climits>
#include <cmath>

#ifdef BOOST_CHARCONV_DEBUG_FIXED
#include <iostream>
#endif

#if (BOOST_CHARCONV_LDBL_BITS == 80 || BOOST_CHARCONV_LDBL_BITS == 128) || defined(BOOST_CHARCONV_HAS_FLOAT128)
#  include <boost/charconv/detail/ryu/ryu_generic_128.hpp>
#  include <boost/charconv/detail/issignaling.hpp>
#endif

namespace boost { namespace charconv { namespace detail {

// ---------------------------------------------------------------------------------------------------------------------
// Floating Point Detail
// ---------------------------------------------------------------------------------------------------------------------

template <typename Real>
inline to_chars_result to_chars_nonfinite(char* first, char* last, Real value, int classification) noexcept;

#if BOOST_CHARCONV_LDBL_BITS == 128 || defined(BOOST_CHARCONV_HAS_STDFLOAT128)

template <typename Real>
inline to_chars_result to_chars_nonfinite(char* first, char* last, Real value, int classification) noexcept
{
    std::ptrdiff_t offset {};
    std::ptrdiff_t buffer_size = last - first;

    if (classification == FP_NAN)
    {
        bool is_negative = false;
        const bool is_signaling = issignaling(value);

        if (std::signbit(value))
        {
            is_negative = true;
            *first++ = '-';
        }

        if (is_signaling && buffer_size >= (9 + static_cast<int>(is_negative)))
        {
            std::memcpy(first, "nan(snan)", 9);
            offset = 9 + static_cast<int>(is_negative);
        }
        else if (is_negative && buffer_size >= 9)
        {
            std::memcpy(first, "nan(ind)", 8);
            offset = 8;
        }
        else if (!is_negative && !is_signaling && buffer_size >= 3)
        {
            std::memcpy(first, "nan", 3);
            offset = 3;
        }
        else
        {
            // Avoid buffer overflow
            return { first, std::errc::result_out_of_range };
        }

    }
    else if (classification == FP_INFINITE)
    {
        auto sign_bit_value = std::signbit(value);
        if (sign_bit_value && buffer_size >= 4)
        {
            std::memcpy(first, "-inf", 4);
            offset = 4;
        }
        else if (!sign_bit_value && buffer_size >= 3)
        {
            std::memcpy(first, "inf", 3);
            offset = 3;
        }
        else
        {
            // Avoid buffer overflow
            return { first, std::errc::result_out_of_range };
        }
    }
    else
    {
        BOOST_UNREACHABLE_RETURN(first);
    }

    return { first + offset, std::errc() };
}

#endif // BOOST_CHARCONV_LDBL_BITS == 128

#ifdef BOOST_CHARCONV_HAS_FLOAT128

// GCC-5 evaluates the following specialization for other types
#if defined(__GNUC__) && __GNUC__ == 5
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

template <>
inline to_chars_result to_chars_nonfinite<__float128>(char* first, char* last, __float128 value, int classification) noexcept
{
    std::ptrdiff_t offset {};
    std::ptrdiff_t buffer_size = last - first;

    IEEEbinary128 bits;
    std::memcpy(&bits, &value, sizeof(value));
    const bool is_negative = bits.sign;

    if (classification == FP_NAN)
    {
        const bool is_signaling = issignaling(value);

        if (is_negative)
        {
            *first++ = '-';
        }

        if (is_signaling && buffer_size >= (9 + static_cast<int>(is_negative)))
        {
            std::memcpy(first, "nan(snan)", 9);
            offset = 9 + static_cast<int>(is_negative);
        }
        else if (is_negative && buffer_size >= 9)
        {
            std::memcpy(first, "nan(ind)", 8);
            offset = 8;
        }
        else if (!is_negative && !is_signaling && buffer_size >= 3)
        {
            std::memcpy(first, "nan", 3);
            offset = 3;
        }
        else
        {
            // Avoid buffer overflow
            return { first, std::errc::result_out_of_range };
        }

    }
    else if (classification == FP_INFINITE)
    {
        if (is_negative && buffer_size >= 4)
        {
            std::memcpy(first, "-inf", 4);
            offset = 4;
        }
        else if (!is_negative && buffer_size >= 3)
        {
            std::memcpy(first, "inf", 3);
            offset = 3;
        }
        else
        {
            // Avoid buffer overflow
            return { first, std::errc::result_out_of_range };
        }
    }
    else
    {
        BOOST_UNREACHABLE_RETURN(first);
    }

    return { first + offset, std::errc() };
}

#if defined(__GNUC__) && __GNUC__ == 5
# pragma GCC diagnostic pop
#endif

#endif // BOOST_CHARCONV_HAS_FLOAT128

template <typename Unsigned_Integer, typename Real, typename std::enable_if<!std::is_same<Unsigned_Integer, uint128>::value, bool>::type = true>
Unsigned_Integer convert_value(Real value) noexcept
{
    Unsigned_Integer temp;
    std::memcpy(&temp, &value, sizeof(Real));
    return temp;
}

template <typename Unsigned_Integer, typename Real, typename std::enable_if<std::is_same<Unsigned_Integer, uint128>::value, bool>::type = true>
Unsigned_Integer convert_value(Real value) noexcept
{
    trivial_uint128 trivial_bits; // NOLINT : Does not need to be init since we memcpy the next step
    std::memcpy(&trivial_bits, &value, sizeof(Real));
    Unsigned_Integer temp {trivial_bits};
    return temp;
}
#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4127) // Conditional expression is constant (BOOST_IF_CONSTEXPR in pre-C++17 modes)
#endif

template <typename Real>
to_chars_result to_chars_hex(char* first, char* last, Real value, int precision) noexcept
{
    // If the user did not specify a precision than we use the maximum representable amount
    // and remove trailing zeros at the end

    int real_precision;
    BOOST_IF_CONSTEXPR (std::is_same<Real, float>::value || std::is_same<Real, double>::value)
    {
        real_precision = precision == -1 ? std::numeric_limits<Real>::max_digits10 : precision;
    }
    else
    {
        #ifdef BOOST_CHARCONV_HAS_FLOAT128
        BOOST_CHARCONV_IF_CONSTEXPR (std::is_same<Real, __float128>::value)
        {
            real_precision = 33;
        }
        else
        #endif
        {
            #if BOOST_CHARCONV_LDBL_BITS == 128
            real_precision = 33;
            #else
            real_precision = 18;
            #endif
        }
    }

    // Sanity check our bounds
    const std::ptrdiff_t buffer_size = last - first;
    if (buffer_size < real_precision || first > last)
    {
        return {last, std::errc::result_out_of_range};
    }

    // Extract the significand and the exponent
    using type_layout = typename std::conditional<std::is_same<Real, float>::value, ieee754_binary32,
                        typename std::conditional<std::is_same<Real, double>::value, ieee754_binary64,
                        #ifdef BOOST_CHARCONV_HAS_FLOAT128
                        typename std::conditional<std::is_same<Real, __float128>::value || BOOST_CHARCONV_LDBL_BITS == 128, ieee754_binary128, ieee754_binary80>::type
                        #elif BOOST_CHARCONV_LDBL_BITS == 128
                        ieee754_binary128
                        #elif BOOST_CHARCONV_LDBL_BITS == 80
                        ieee754_binary80
                        #else
                        ieee754_binary64
                        #endif
                        >::type>::type;

    #ifdef BOOST_CHARCONV_HAS_INT128
    using Unsigned_Integer = typename std::conditional<std::is_same<Real, float>::value, std::uint32_t,
                             typename std::conditional<std::is_same<Real, double>::value, std::uint64_t, boost::uint128_type>::type>::type;
    #else
    using Unsigned_Integer = typename std::conditional<std::is_same<Real, float>::value, std::uint32_t,
                             typename std::conditional<std::is_same<Real, double>::value, std::uint64_t, uint128>::type>::type;
    #endif

    Unsigned_Integer uint_value {convert_value<Unsigned_Integer>(value)};

    const Unsigned_Integer denorm_mask = (Unsigned_Integer(1) << (type_layout::significand_bits)) - 1;
    const Unsigned_Integer significand = uint_value & denorm_mask;
    auto exponent = static_cast<std::int32_t>(uint_value >> type_layout::significand_bits);
    BOOST_IF_CONSTEXPR (!(std::is_same<Real, float>::value || std::is_same<Real, double>::value))
    {
        exponent += 2;
    }

    // Align the significand to the hexit boundaries (i.e. divisible by 4)
    constexpr auto hex_precision = std::is_same<Real, float>::value ? 6 : std::is_same<Real, double>::value ? 13 : 28;
    constexpr auto nibble_bits = CHAR_BIT / 2;
    constexpr auto hex_bits = hex_precision * nibble_bits;
    const Unsigned_Integer hex_mask = (static_cast<Unsigned_Integer>(1) << hex_bits) - 1;

    Unsigned_Integer aligned_significand;
    BOOST_IF_CONSTEXPR (std::is_same<Real, float>::value)
    {
        aligned_significand = significand << 1;
    }
    else
    {
        aligned_significand = significand;
    }

    // Adjust the exponent based on the bias as described in IEEE 754
    std::int32_t unbiased_exponent;
    if (exponent == 0 && significand != 0)
    {
        // Subnormal value since we already handled zero
        unbiased_exponent = 1 + type_layout::exponent_bias;
    }
    else
    {
        aligned_significand |= static_cast<Unsigned_Integer>(1) << hex_bits;
        unbiased_exponent = exponent + type_layout::exponent_bias;
    }

    // Bounds check the exponent
    BOOST_IF_CONSTEXPR (std::is_same<Real, float>::value)
    {
        if (unbiased_exponent > 127)
        {
            unbiased_exponent -= 256;
        }
    }
    else BOOST_IF_CONSTEXPR (std::is_same<Real, double>::value)
    {
        if (unbiased_exponent > 1023)
        {
            unbiased_exponent -= 2048;
        }
    }
    else
    {
        if (unbiased_exponent > 16383)
        {
            unbiased_exponent -= 32768;
        }
    }

    const std::uint32_t abs_unbiased_exponent = unbiased_exponent < 0 ? static_cast<std::uint32_t>(-unbiased_exponent) : 
                                                                        static_cast<std::uint32_t>(unbiased_exponent);
    
    // Bounds check
    // Sign + integer part + '.' + precision of fraction part + p+/p- + exponent digits
    const std::ptrdiff_t total_length = (value < 0) + 2 + real_precision + 2 + num_digits(abs_unbiased_exponent);
    if (total_length > buffer_size)
    {
        return {last, std::errc::result_out_of_range};
    }

    // Round if required
    if (real_precision < hex_precision)
    {
        const int lost_bits = (hex_precision - real_precision) * nibble_bits;
        const Unsigned_Integer lsb_bit = aligned_significand;
        const Unsigned_Integer round_bit = aligned_significand << 1;
        const Unsigned_Integer tail_bit = round_bit - 1;
        const Unsigned_Integer round = round_bit & (tail_bit | lsb_bit) & (static_cast<Unsigned_Integer>(1) << lost_bits);
        aligned_significand += round;
    }

    // Print the sign
    if (value < 0)
    {
        *first++ = '-';
    }

    // Print the integral part
    const auto leading_nibble = static_cast<std::uint32_t>(aligned_significand >> hex_bits);
    *first++ = static_cast<char>('0' + leading_nibble);
    aligned_significand &= hex_mask;

    // Print the fractional part
    if (real_precision > 0)
    {
        *first++ = '.';
        std::int32_t remaining_bits = hex_bits;

        while (true)
        {
            remaining_bits -= nibble_bits;
            const auto current_nibble = static_cast<std::uint32_t>(aligned_significand >> remaining_bits);
            *first++ = digit_table[current_nibble];

            --real_precision;
            if (real_precision == 0)
            {
                break;
            }
            else if (remaining_bits == 0)
            {
                // Do not print trailing zeros with unspecified precision
                if (precision != -1)
                {
                    std::memset(first, '0', static_cast<std::size_t>(real_precision));
                    first += real_precision;
                }
                break;
            }

            // Mask away the hexit we just printed
            aligned_significand &= (static_cast<Unsigned_Integer>(1) << remaining_bits) - 1;
        }
    }

    // Remove any trailing zeros if the precision was unspecified
    if (precision == -1)
    {
        --first;
        while (*first == '0')
        {
            --first;
        }
        ++first;
    }

    // Print the exponent
    *first++ = 'p';
    if (unbiased_exponent < 0)
    {
        *first++ = '-';
    }
    else
    {
        *first++ = '+';
    }

    return to_chars_int(first, last, abs_unbiased_exponent);
}

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif

template <typename Real>
to_chars_result to_chars_fixed_impl(char* first, char* last, Real value, int precision = -1) noexcept
{
    const std::ptrdiff_t buffer_size = last - first;

    auto abs_value = std::abs(value);

    auto value_struct = boost::charconv::detail::to_decimal(value);
    if (value_struct.is_negative)
    {
        *first++ = '-';
    }

    int num_dig = 0;
    if (precision != -1)
    {
        num_dig = num_digits(value_struct.significand);
        while (num_dig > precision + 2)
        {
            value_struct.significand /= 10;
            ++value_struct.exponent;
            --num_dig;
        }

        if (num_dig == precision + 2)
        {
            const auto trailing_dig = value_struct.significand % 10;
            value_struct.significand /= 10;
            ++value_struct.exponent;
            --num_dig;

            if (trailing_dig >= 5)
            {
                ++value_struct.significand;
            }
        }

        if (value_struct.significand % 10 == 0)
        {
            value_struct.significand /= 10;
            ++value_struct.exponent;
            --num_dig;
        }
    }

    auto r = to_chars_integer_impl(first, last, value_struct.significand);
    if (r.ec != std::errc())
    {
        return r;
    }

    // Bounds check
    if (abs_value >= 1)
    {
        if (value_struct.exponent < 0 && -value_struct.exponent < buffer_size)
        {
            std::memmove(r.ptr + value_struct.exponent + 1, r.ptr + value_struct.exponent,
                         static_cast<std::size_t>(-value_struct.exponent));
            std::memset(r.ptr + value_struct.exponent, '.', 1);
            ++r.ptr;
        }

        while (std::fmod(abs_value, 10) == 0)
        {
            *r.ptr++ = '0';
            abs_value /= 10;
        }
    }
    else
    {
        #ifdef BOOST_CHARCONV_DEBUG_FIXED
        std::cerr << "Value: " << value
                  << "\n  Buf: " << first
                  << "\n  sig: " << value_struct.significand
                  << "\n  exp: " << value_struct.exponent << std::endl;
        #endif

        const std::size_t offset_bytes = -value_struct.exponent - num_dig;

        std::memmove(first + 2 + static_cast<std::size_t>(value_struct.is_negative) + offset_bytes,
                     first + static_cast<std::size_t>(value_struct.is_negative),
                     static_cast<std::size_t>(-value_struct.exponent - offset_bytes));

        std::memcpy(first + static_cast<std::size_t>(value_struct.is_negative), "0.", 2U);
        first += 2;
        r.ptr += 2;

        while (num_dig < -value_struct.exponent)
        {
            *first++ = '0';
            ++num_dig;
            ++r.ptr;
        }
    }

    return { r.ptr, std::errc() };
}

template <typename Real>
to_chars_result to_chars_float_impl(char* first, char* last, Real value, chars_format fmt = chars_format::general, int precision = -1 ) noexcept
{
    using Unsigned_Integer = typename std::conditional<std::is_same<Real, double>::value, std::uint64_t, std::uint32_t>::type;

    auto abs_value = std::abs(value);
    constexpr auto max_fractional_value = std::is_same<Real, double>::value ? static_cast<Real>(1e16) : static_cast<Real>(1e7);
    constexpr auto min_fractional_value = 1 / max_fractional_value;
    constexpr auto max_value = static_cast<Real>(std::numeric_limits<Unsigned_Integer>::max());

    // Unspecified precision so we always go with the shortest representation
    if (precision == -1)
    {
        if (fmt == boost::charconv::chars_format::general || fmt == boost::charconv::chars_format::fixed)
        {
            if (abs_value >= 1 && abs_value < max_fractional_value)
            {
                return to_chars_fixed_impl(first, last, value, precision);
            }
            else if (abs_value >= max_fractional_value && abs_value < max_value)
            {
                if (value < 0)
                {
                    *first++ = '-';
                }
                return to_chars_integer_impl(first, last, static_cast<std::uint64_t>(abs_value));
            }
            else
            {
                auto* ptr = boost::charconv::detail::to_chars(value, first, fmt);
                return { ptr, std::errc() };
            }
        }
        else if (fmt == boost::charconv::chars_format::scientific)
        {
            auto* ptr = boost::charconv::detail::to_chars(value, first, fmt);
            return { ptr, std::errc() };
        }
    }
    else
    {
        if (fmt != boost::charconv::chars_format::hex)
        {
            if (abs_value >= min_fractional_value && abs_value < max_fractional_value)
            {
                return to_chars_fixed_impl(first, last, value, precision);
            }

            auto* ptr = boost::charconv::detail::floff<boost::charconv::detail::main_cache_full, boost::charconv::detail::extended_cache_long>(value, precision, first, fmt);
            return { ptr, std::errc() };
        }
    }

    // Before passing to hex check for edge cases
    BOOST_ATTRIBUTE_UNUSED char* ptr;
    const int classification = std::fpclassify(value);
    switch (classification)
    {
        case FP_INFINITE:
        case FP_NAN:
            // The dragonbox impl will return the correct type of NaN
            ptr = boost::charconv::detail::to_chars(value, first, chars_format::general);
            return { ptr, std::errc() };
        case FP_ZERO:
            if (std::signbit(value))
            {
                *first++ = '-';
            }
            std::memcpy(first, "0p+0", 4); // NOLINT : No null terminator is purposeful
            return {first + 4, std::errc()};
        default:
            // Do nothing
            (void)ptr;
    }

    // Hex handles both cases already
    return boost::charconv::detail::to_chars_hex(first, last, value, precision);
}

#ifdef BOOST_CHARCONV_HAS_FLOAT128
inline int print_val(char* first, std::size_t size, char* format, __float128 value) noexcept
{
    return quadmath_snprintf(first, size, format, value);
}
#endif

template <typename T>
inline int print_val(char* first, std::size_t size, char* format, T value) noexcept
{
    return std::snprintf(first, size, format, value);
}

template <typename T>
to_chars_result to_chars_printf_impl(char* first, char* last, T value, chars_format fmt, int precision)
{
    // v % + . + num_digits(INT_MAX) + specifier + null terminator
    // 1 + 1 + 10 + 1 + 1
    char format[14] {};
    std::memcpy(format, "%", 1); // NOLINT : No null terminator is purposeful
    std::size_t pos = 1;

    // precision of -1 is unspecified
    if (precision != -1 && fmt != chars_format::fixed)
    {
        format[pos] = '.';
        ++pos;
        const auto unsigned_precision = static_cast<std::uint32_t>(precision);
        if (unsigned_precision < 10)
        {
            boost::charconv::detail::print_1_digit(unsigned_precision, format + pos);
            ++pos;
        }
        else if (unsigned_precision < 100)
        {
            boost::charconv::detail::print_2_digits(unsigned_precision, format + pos);
            pos += 2;
        }
        else
        {
            boost::charconv::detail::to_chars_int(format + pos, format + sizeof(format), precision);
            pos = std::strlen(format);
        }
    }
    else if (fmt == chars_format::fixed)
    {
        // Force 0 decimal places
        std::memcpy(format + pos, ".0", 2); // NOLINT : No null terminator is purposeful
        pos += 2;
    }

    // Add the type identifier
    #ifdef BOOST_CHARCONV_HAS_FLOAT128
    BOOST_CHARCONV_IF_CONSTEXPR (std::is_same<T, __float128>::value || std::is_same<T, long double>::value)
    {
        format[pos] = std::is_same<T, __float128>::value ? 'Q' : 'L';
        ++pos;
    }
    #else
    BOOST_CHARCONV_IF_CONSTEXPR (std::is_same<T, long double>::value)
    {
        format[pos] = 'L';
        ++pos;
    }
    #endif

    // Add the format character
    switch (fmt)
    {
        case boost::charconv::chars_format::general:
            format[pos] = 'g';
            break;

        case boost::charconv::chars_format::scientific:
            format[pos] = 'e';
            break;

        case boost::charconv::chars_format::fixed:
            format[pos] = 'f';
            break;

        case boost::charconv::chars_format::hex:
            format[pos] = 'a';
            break;
    }

    const auto rv = print_val(first, static_cast<std::size_t>(last - first), format, value);

    if (rv <= 0)
    {
        return {last, static_cast<std::errc>(errno)};
    }

    return {first + rv, std::errc()};
}

} // Namespace detail

// integer overloads
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, bool value, int base) noexcept = delete;
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, char value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, signed char value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, unsigned char value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, short value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, unsigned short value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, int value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, unsigned int value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, long value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, unsigned long value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, long long value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, unsigned long long value, int base = 10) noexcept
{
    return detail::to_chars_int(first, last, value, base);
}

#ifdef BOOST_CHARCONV_HAS_INT128
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, boost::int128_type value, int base = 10) noexcept
{
    return detail::to_chars128(first, last, value, base);
}
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, boost::uint128_type value, int base = 10) noexcept
{
    return detail::to_chars128(first, last, value, base);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
// Floating Point
//----------------------------------------------------------------------------------------------------------------------

BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, float value,
                                             chars_format fmt = chars_format::general, int precision = -1 ) noexcept;
BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, double value, 
                                             chars_format fmt = chars_format::general, int precision = -1 ) noexcept;
BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, long double value,
                                             chars_format fmt = chars_format::general, int precision = -1 ) noexcept;

#ifdef BOOST_CHARCONV_HAS_FLOAT128
BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, __float128 value,
                                             chars_format fmt = chars_format::general, int precision = -1 ) noexcept;
#endif

#ifdef BOOST_CHARCONV_HAS_FLOAT16
BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, std::float16_t value, 
                                             chars_format fmt = chars_format::general, int precision = -1 ) noexcept;
#endif
#ifdef BOOST_CHARCONV_HAS_FLOAT32
BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, std::float32_t value, 
                                             chars_format fmt = chars_format::general, int precision = -1 ) noexcept;
#endif
#ifdef BOOST_CHARCONV_HAS_FLOAT64
BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, std::float64_t value, 
                                             chars_format fmt = chars_format::general, int precision = -1 ) noexcept;
#endif
#if defined(BOOST_CHARCONV_HAS_STDFLOAT128) && defined(BOOST_CHARCONV_HAS_FLOAT128)
BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, std::float128_t value,
                                             chars_format fmt = chars_format::general, int precision = -1 ) noexcept;
#endif
#ifdef BOOST_CHARCONV_HAS_BRAINFLOAT16
BOOST_CHARCONV_DECL to_chars_result to_chars(char* first, char* last, std::bfloat16_t value, 
                                             chars_format fmt = chars_format::general, int precision = -1 ) noexcept;
#endif

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_TO_CHARS_HPP_INCLUDED
