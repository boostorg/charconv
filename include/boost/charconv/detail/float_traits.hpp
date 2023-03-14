// Copyright 2020-2023 Junekey Jeon
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_FLOAT_TRAITS_HPP
#define BOOST_CHARCONV_DETAIL_FLOAT_TRAITS_HPP

#include <boost/charconv/detail/config.hpp>
#include <type_traits>
#include <limits>
#include <climits>
#include <cstddef>
#include <cstring>

namespace boost { namespace charconv { namespace detail {

template <typename T>
struct physical_bits
{
    constexpr std::size_t value = sizeof(T) * CHAR_BIT;
};

template <typename T>
struct value_bits
{
    constexpr std::size_t value_bits = std::numeric_limits<typename std::enable_if<std::is_unsigned<T>::value, T>::type>::digits;
};

// A floating-point traits class defines ways to interpret a bit pattern of given size as an
// encoding of floating-point number. This is a default implementation of such a traits class,
// supporting ways to interpret 32-bits into a binary32-encoded floating-point number and to
// interpret 64-bits into a binary64-encoded floating-point number. Users might specialize this
// class to change the default behavior for certain types.
template <typename T>
struct default_float_traits 
{
    // I don't know if there is a truly reliable way of detecting
    // IEEE-754 binary32/binary64 formats; I just did my best here.
    static_assert(std::numeric_limits<T>::is_iec559 && std::numeric_limits<T>::radix == 2 &&
                        (detail::physical_bits<T>::value == 32 || detail::physical_bits<T>::value == 64),
                    "default_ieee754_traits only works for 32-bits or 64-bits types "
                    "supporting binary32 or binary64 formats!");

    // The type that is being viewed.
    using type = T;

    // Refers to the format specification class.
    using format = typename std::conditional<detail::physical_bits<T>::value == 32, ieee754_binary32, ieee754_binary64>::type;

    // Defines an unsigned integer type that is large enough to carry a variable of type T.
    // Most of the operations will be done on this integer type.
    using carrier_uint = typename std::conditional<detail::physical_bits<T>::value == 32, std::uint32_t, std::uint64_t>::type;
    static_assert(sizeof(carrier_uint) == sizeof(T));

    // Number of bits in the above unsigned integer type.
    static constexpr int carrier_bits = static_cast<int>(detail::physical_bits<carrier_uint>);

    // Convert from carrier_uint into the original type.
    // Depending on the floating-point encoding format, this operation might not be possible for
    // some specific bit patterns. However, the contract is that u always denotes a
    // valid bit pattern, so this function must be assumed to be noexcept.
    static T carrier_to_float(carrier_uint u) noexcept 
    {
        T x;
        std::memcpy(&x, &u, sizeof(carrier_uint));
        return x;
    }

    // Same as above.
    static carrier_uint float_to_carrier(T x) noexcept 
    {
        carrier_uint u;
        std::memcpy(&u, &x, sizeof(carrier_uint));
        return u;
    }

    // Extract exponent bits from a bit pattern.
    // The result must be aligned to the LSB so that there is no additional zero paddings
    // on the right. This function does not do bias adjustment.
    BOOST_CHARCONV_CXX14_CONSTEXPR unsigned extract_exponent_bits(carrier_uint u) const noexcept 
    {
        constexpr int significand_bits = format::significand_bits;
        constexpr int exponent_bits = format::exponent_bits;

        static_assert(detail::value_bits<unsigned int> > exponent_bits);
        constexpr auto exponent_bits_mask = (static_cast<unsigned int>(1) << exponent_bits) - 1;

        return static_cast<unsigned int>(u >> significand_bits) & exponent_bits_mask;
    }

    // Extract significand bits from a bit pattern.
    // The result must be aligned to the LSB so that there is no additional zero paddings
    // on the right. The result does not contain the implicit bit.
    BOOST_CHARCONV_CXX14_CONSTEXPR carrier_uint extract_significand_bits(carrier_uint u) noexcept 
    {
        constexpr auto mask = carrier_uint((carrier_uint(1) << format::significand_bits) - 1);
        return carrier_uint(u & mask);
    }

    // Remove the exponent bits and extract significand bits together with the sign bit.
    constexpr carrier_uint remove_exponent_bits(carrier_uint u, unsigned int exponent_bits) noexcept 
    {
        return u ^ (carrier_uint(exponent_bits) << format::significand_bits);
    }

    // Shift the obtained signed significand bits to the left by 1 to remove the sign bit.
    constexpr carrier_uint remove_sign_bit_and_shift(carrier_uint u) noexcept 
    {
        return carrier_uint(u << 1);
    }

    // The actual value of exponent is obtained by adding this value to the extracted exponent
    // bits.
    static constexpr int exponent_bias = 1 - (1 << (carrier_bits - format::significand_bits - 2));

    // Obtain the actual value of the binary exponent from the extracted exponent bits.
    BOOST_CHARCONV_CXX14_CONSTEXPR int binary_exponent(unsigned int exponent_bits) const noexcept 
    {
        if (exponent_bits == 0) 
        {
            return format::min_exponent;
        }
        else 
        {
            return static_cast<int>(exponent_bits) + format::exponent_bias;
        }
    }

    // Obtain the actual value of the binary exponent from the extracted significand bits and
    // exponent bits.
    BOOST_CHARCONV_CXX14_CONSTEXPR carrier_uint binary_significand(carrier_uint significand_bits, unsigned int exponent_bits) noexcept 
    {
        if (exponent_bits == 0) 
        {
            return significand_bits;
        }
        else 
        {
            return significand_bits | (static_cast<carrier_uint>(1) << format::significand_bits);
        }
    }


    /* Various boolean observer functions */

    constexpr bool is_nonzero(carrier_uint u) noexcept 
    { 
        return (u << 1) != 0; 
    }

    BOOST_CHARCONV_CXX14_CONSTEXPR bool is_positive(carrier_uint u) noexcept 
    {
        constexpr auto sign_bit = carrier_uint(1) << (format::significand_bits + format::exponent_bits);
        return u < sign_bit;
    }

    BOOST_CHARCONV_CXX14_CONSTEXPR bool is_negative(carrier_uint u) noexcept 
    { 
        return !is_positive(u);
    }

    BOOST_CHARCONV_CXX14_CONSTEXPR bool is_finite(unsigned int exponent_bits) noexcept
    {
        constexpr unsigned int exponent_bits_all_set = (1u << format::exponent_bits) - 1;
        return exponent_bits != exponent_bits_all_set;
    }

    constexpr bool has_all_zero_significand_bits(carrier_uint u) noexcept
    {
        return (u << 1) == 0;
    }

    constexpr bool has_even_significand_bits(carrier_uint u) noexcept
    {
        return u % 2 == 0;
    }
};

#ifdef BOOST_NO_CXX17_INLINE_VARIABLES
constexpr int default_float_traits<T>::exponent_bias;
#endif

// Convenient wrappers for floating-point traits classes.
// In order to reduce the argument passing overhead, these classes should be as simple as
// possible (e.g., no inheritance, no private non-static data member, etc.; this is an
// unfortunate fact about common ABI convention).

template <typename T, typename Traits = default_float_traits<T>>
struct float_bits;

template <typename T, typename Traits = default_float_traits<T>>
struct signed_significand_bits;

template <typename T, typename Traits>
struct float_bits
{
    using type = T;
    using traits_type = Traits;
    using carrier_uint = typename traits_type::carrier_uint;

    carrier_uint u;

    float_bits() = default;
    constexpr explicit float_bits(carrier_uint bit_pattern) noexcept : u{bit_pattern} {}
    constexpr explicit float_bits(T float_value) noexcept
        : u{traits_type::float_to_carrier(float_value)} {}

    constexpr T to_float() const noexcept { return traits_type::carrier_to_float(u); }

    // Extract exponent bits from a bit pattern.
    // The result must be aligned to the LSB so that there is no additional zero paddings
    // on the right. This function does not do bias adjustment.
    constexpr unsigned int extract_exponent_bits() const noexcept
    {
        return traits_type::extract_exponent_bits(u);
    }

    // Extract significand bits from a bit pattern.
    // The result must be aligned to the LSB so that there is no additional zero paddings
    // on the right. The result does not contain the implicit bit.
    constexpr carrier_uint extract_significand_bits() const noexcept
    {
        return traits_type::extract_significand_bits(u);
    }

    // Remove the exponent bits and extract significand bits together with the sign bit.
    constexpr signed_significand_bits<type, traits_type> remove_exponent_bits(unsigned exponent_bits) const noexcept
    {
        return signed_significand_bits<type, traits_type>(traits_type::remove_exponent_bits(u, exponent_bits));
    }

    // Obtain the actual value of the binary exponent from the extracted exponent bits.
    constexpr int binary_exponent(unsigned exponent_bits) noexcept
    {
        return traits_type::binary_exponent(exponent_bits);
    }
    constexpr int binary_exponent() const noexcept
    {
        return binary_exponent(extract_exponent_bits());
    }

    // Obtain the actual value of the binary exponent from the extracted significand bits and
    // exponent bits.
    constexpr carrier_uint binary_significand(carrier_uint significand_bits, unsigned exponent_bits) noexcept 
    {
        return traits_type::binary_significand(significand_bits, exponent_bits);
    }
    constexpr carrier_uint binary_significand() const noexcept 
    {
        return binary_significand(extract_significand_bits(), extract_exponent_bits());
    }

    constexpr bool is_nonzero() const noexcept { return traits_type::is_nonzero(u); }
    constexpr bool is_positive() const noexcept { return traits_type::is_positive(u); }
    constexpr bool is_negative() const noexcept { return traits_type::is_negative(u); }
    constexpr bool is_finite(unsigned exponent_bits) const noexcept 
    {
        return traits_type::is_finite(exponent_bits);
    }
    constexpr bool is_finite() const noexcept
    {
        return traits_type::is_finite(extract_exponent_bits());
    }
    constexpr bool has_even_significand_bits() const noexcept
    {
        return traits_type::has_even_significand_bits(u);
    }
};

template <typename T, typename Traits>
struct signed_significand_bits 
{
    using type = T;
    using traits_type = Traits;
    using carrier_uint = typename traits_type::carrier_uint;

    carrier_uint u;

    signed_significand_bits() = default;
    constexpr explicit signed_significand_bits(carrier_uint bit_pattern) noexcept
        : u{bit_pattern} {}

    // Shift the obtained signed significand bits to the left by 1 to remove the sign bit.
    constexpr carrier_uint remove_sign_bit_and_shift() const noexcept 
    {
        return traits_type::remove_sign_bit_and_shift(u);
    }

    constexpr bool is_positive() const noexcept { return traits_type::is_positive(u); }
    constexpr bool is_negative() const noexcept { return traits_type::is_negative(u); }
    constexpr bool has_all_zero_significand_bits() const noexcept 
    {
        return traits_type::has_all_zero_significand_bits(u);
    }
    constexpr bool has_even_significand_bits() const noexcept
    {
        return traits_type::has_even_significand_bits(u);
    }
};

template <typename Unsigned_Integer, bool is_signed, bool trailing_zero_flag>
struct decimal_fp;

template <typename Unsigned_Integer>
struct decimal_fp<Unsigned_Integer, false, false> 
{
    using carrier_uint = Unsigned_Integer;

    carrier_uint significand;
    int exponent;
};

template <typename Unsigned_Integer>
struct decimal_fp<Unsigned_Integer, true, false> 
{
    using carrier_uint = Unsigned_Integer;

    carrier_uint significand;
    int exponent;
    bool is_negative;
};

template <typename Unsigned_Integer>
struct decimal_fp<Unsigned_Integer, false, true> 
{
    using carrier_uint = Unsigned_Integer;

    carrier_uint significand;
    int exponent;
    bool may_have_trailing_zeros;
};

template <typename Unsigned_Integer>
struct decimal_fp<Unsigned_Integer, true, true> 
{
    using carrier_uint = Unsigned_Integer;

    carrier_uint significand;
    int exponent;
    bool is_negative;
    bool may_have_trailing_zeros;
};

template <typename Unsigned_Integer>
using unsigned_decimal_fp = decimal_fp<Unsigned_Integer, false, false>;

template <typename Unsigned_Integer>
using signed_decimal_fp = decimal_fp<Unsigned_Integer, true, false>;

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_FLOAT_TRAITS_HPP
