// Copyright 2020-2023 Daniel Lemire
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// Derivative of: https://github.com/fastfloat/fast_float

#ifndef BOOST_CHARCONV_DETAIL_FAST_FLOAT_FLOAT_COMMON_HPP
#define BOOST_CHARCONV_DETAIL_FAST_FLOAT_FLOAT_COMMON_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/chars_format.hpp>
#include <boost/core/bit.hpp>
#include <type_traits>
#include <cstddef>
#include <cstdint>

#ifndef BOOST_CHARCONV_HAS_STD_BITCAST
#  include <boost/core/bit.hpp>
#endif

namespace boost { namespace charconv { namespace detail { namespace fast_float {

template <typename UC>
struct parse_options_t 
{
    constexpr explicit parse_options_t(chars_format fmt = chars_format::general, UC dot = static_cast<UC>('.'))
        : format(fmt), decimal_point(dot) {}

    /** Which number formats are accepted */
    chars_format format;
    /** The character used as decimal point */
    UC decimal_point;
};
using parse_options = parse_options_t<char>;

template <typename UC>
BOOST_CHARCONV_CXX14_CONSTEXPR bool fastfloat_strncasecmp(UC const * input1, UC const * input2, std::size_t length)
{
    char running_diff {0};
    for (std::size_t i = 0; i < length; ++i)
    {
        running_diff |= (static_cast<char>(input1[i]) ^ static_cast<char>(input2[i]));
    }

    return (running_diff == 0) || (running_diff == 32);
}

// a pointer and a length to a contiguous block of memory
template <typename T>
struct span 
{
    const T* ptr;
    std::size_t length;

    constexpr span(const T* _ptr, size_t _length) : ptr(_ptr), length(_length) {}
    constexpr span() : ptr(nullptr), length(0) {}

    constexpr std::size_t len() const noexcept
    {
        return length;
    }

    BOOST_CHARCONV_CXX14_CONSTEXPR const T& operator[](std::size_t index) const noexcept
    {
        BOOST_CHARCONV_DEBUG_ASSERT(index < length);
        return ptr[index];
    }
};

// slow emulation routine for 32-bit
BOOST_FORCEINLINE constexpr std::uint64_t emulu(std::uint32_t x, std::uint32_t y) 
{
    return x * static_cast<uint64_t>(y);
}

struct adjusted_mantissa
{
    std::uint64_t mantissa{0};
    std::int32_t power2{0}; // a negative value indicates an invalid result
    adjusted_mantissa() = default;

    constexpr bool operator==(const adjusted_mantissa &o) const 
    {
        return mantissa == o.mantissa && power2 == o.power2;
    }
    constexpr bool operator!=(const adjusted_mantissa &o) const 
    {
        return mantissa != o.mantissa || power2 != o.power2;
    }
};

// Bias so we can get the real exponent with an invalid adjusted_mantissa.
static constexpr std::int32_t invalid_am_bias = -0x8000;

// used for binary_format_lookup_tables<T>::max_mantissa
constexpr std::uint64_t constant_55555 = 5 * 5 * 5 * 5 * 5;

template <typename T, typename U = void>
struct binary_format_lookup_tables;

template <typename T> 
struct binary_format : binary_format_lookup_tables<T>
{
    using equiv_uint = typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;

    static inline constexpr int mantissa_explicit_bits();
    static inline constexpr int minimum_exponent();
    static inline constexpr int infinite_power();
    static inline constexpr int sign_index();
    static inline constexpr int min_exponent_fast_path(); // used when fegetround() == FE_TONEAREST
    static inline constexpr int max_exponent_fast_path();
    static inline constexpr int max_exponent_round_to_even();
    static inline constexpr int min_exponent_round_to_even();
    static inline constexpr std::uint64_t max_mantissa_fast_path(std::int64_t power);
    static inline constexpr std::uint64_t max_mantissa_fast_path(); // used when fegetround() == FE_TONEAREST
    static inline constexpr int largest_power_of_ten();
    static inline constexpr int smallest_power_of_ten();
    static inline constexpr T exact_power_of_ten(std::int64_t power);
    static inline constexpr size_t max_digits();
    static inline constexpr equiv_uint exponent_mask();
    static inline constexpr equiv_uint mantissa_mask();
    static inline constexpr equiv_uint hidden_bit_mask();
};

template <typename U>
struct binary_format_lookup_tables<double, U> 
{
    static constexpr double powers_of_ten[] = {
        1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
        1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

    // Largest integer value v so that (5**index * v) <= 1<<53.
    // 0x10000000000000 == 1 << 53
    static constexpr std::uint64_t max_mantissa[] = {
        UINT64_C(0x10000000000000),
        UINT64_C(0x10000000000000) / UINT64_C(5),
        UINT64_C(0x10000000000000) / (UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (UINT64_C(5) * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555),
        UINT64_C(0x10000000000000) / (constant_55555 * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * UINT64_C(5) * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555 * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555 * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555 * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555 * UINT64_C(5) * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555 * constant_55555),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555 * constant_55555 * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555 * constant_55555 * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555 * constant_55555 * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
        UINT64_C(0x10000000000000) / (constant_55555 * constant_55555 * constant_55555 * constant_55555 * UINT64_C(5) * UINT64_C(5) * UINT64_C(5) * UINT64_C(5))};
};

#if defined(BOOST_NO_CXX17_INLINE_VARIABLES) && BOOST_MSVC != 1900
template <typename U>
constexpr double binary_format_lookup_tables<double, U>::powers_of_ten[];

template <typename U>
constexpr uint64_t binary_format_lookup_tables<double, U>::max_mantissa[];
#endif

template <typename U>
struct binary_format_lookup_tables<float, U>
{
    static constexpr float powers_of_ten[] = {1e0f, 1e1f, 1e2f, 1e3f, 1e4f, 1e5f, 1e6f, 1e7f, 1e8f, 1e9f, 1e10f};

    // Largest integer value v so that (5**index * v) <= 1<<24.
    // 0x1000000 == 1<<24
    static constexpr uint64_t max_mantissa[] = {
            UINT64_C(0x1000000),
            UINT64_C(0x1000000) / UINT64_C(5),
            UINT64_C(0x1000000) / (UINT64_C(5) * UINT64_C(5)),
            UINT64_C(0x1000000) / (UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
            UINT64_C(0x1000000) / (UINT64_C(5) * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
            UINT64_C(0x1000000) / (constant_55555),
            UINT64_C(0x1000000) / (constant_55555 * UINT64_C(5)),
            UINT64_C(0x1000000) / (constant_55555 * UINT64_C(5) * UINT64_C(5)),
            UINT64_C(0x1000000) / (constant_55555 * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
            UINT64_C(0x1000000) / (constant_55555 * UINT64_C(5) * UINT64_C(5) * UINT64_C(5) * UINT64_C(5)),
            UINT64_C(0x1000000) / (constant_55555 * constant_55555),
            UINT64_C(0x1000000) / (constant_55555 * constant_55555 * UINT64_C(5))};
};

#if defined(BOOST_NO_CXX17_INLINE_VARIABLES) && BOOST_MSVC != 1900
template <typename U>
constexpr float binary_format_lookup_tables<float, U>::powers_of_ten[];

template <typename U>
constexpr uint64_t binary_format_lookup_tables<float, U>::max_mantissa[];
#endif

template <> 
inline constexpr int binary_format<double>::min_exponent_fast_path()
{
    #if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
    
    return 0;
    
    #else
    
    return -22;
    
    #endif
}

template <> 
inline constexpr int binary_format<float>::min_exponent_fast_path()
{
    #if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
    
    return 0;
    
    #else
  
    return -10;

    #endif
}

template <> 
inline constexpr int binary_format<double>::mantissa_explicit_bits()
{
    return 52;
}
template <> 
inline constexpr int binary_format<float>::mantissa_explicit_bits()
{
    return 23;
}

template <> 
inline constexpr int binary_format<double>::max_exponent_round_to_even()
{
    return 23;
}
template <> 
inline constexpr int binary_format<float>::max_exponent_round_to_even() 
{
    return 10;
}

template <> 
inline constexpr int binary_format<double>::min_exponent_round_to_even() 
{
    return -4;
}
template <> 
inline constexpr int binary_format<float>::min_exponent_round_to_even() 
{
    return -17;
}

template <> 
inline constexpr int binary_format<double>::minimum_exponent() 
{
    return -1023;
}
template <> inline constexpr int binary_format<float>::minimum_exponent()
{
    return -127;
}

template <> 
inline constexpr int binary_format<double>::infinite_power() 
{
    return 0x7FF;
}
template <> 
inline constexpr int binary_format<float>::infinite_power() 
{
    return 0xFF;
}

template <> 
inline constexpr int binary_format<double>::sign_index() 
{
    return 63; 
}
template <> 
inline constexpr int binary_format<float>::sign_index() 
{ 
    return 31; 
}

template <> 
inline constexpr int binary_format<double>::max_exponent_fast_path() 
{
    return 22;
}
template <> 
inline constexpr int binary_format<float>::max_exponent_fast_path() 
{
    return 10;
}

template <> 
inline constexpr std::uint64_t binary_format<double>::max_mantissa_fast_path()
{
    return static_cast<uint64_t>(2) << mantissa_explicit_bits();
}
template <> 
inline constexpr std::uint64_t binary_format<double>::max_mantissa_fast_path(std::int64_t power)
{
    // caller is responsible to ensure that
    // power >= 0 && power <= 22
    //
    // Work around clang bug https://godbolt.org/z/zedh7rrhc
    return static_cast<void>(max_mantissa[0]), max_mantissa[power];
}

template <> 
inline constexpr std::uint64_t binary_format<float>::max_mantissa_fast_path()
{
    return static_cast<std::uint64_t>(2) << mantissa_explicit_bits();
}
template <> 
inline constexpr uint64_t binary_format<float>::max_mantissa_fast_path(std::int64_t power)
{
    // caller is responsible to ensure that
    // power >= 0 && power <= 10
    //
    // Work around clang bug https://godbolt.org/z/zedh7rrhc
    return static_cast<void>(max_mantissa[0]), max_mantissa[power];
}

template <>
inline constexpr double binary_format<double>::exact_power_of_ten(std::int64_t power) 
{
    // Work around clang bug https://godbolt.org/z/zedh7rrhc
    return static_cast<void>(powers_of_ten[0]), powers_of_ten[power];
}
template <>
inline constexpr float binary_format<float>::exact_power_of_ten(std::int64_t power)
{
    // Work around clang bug https://godbolt.org/z/zedh7rrhc
    return static_cast<void>(powers_of_ten[0]), powers_of_ten[power];
}

template <>
inline constexpr int binary_format<double>::largest_power_of_ten()
{
    return 308;
}
template <>
inline constexpr int binary_format<float>::largest_power_of_ten()
{
    return 38;
}

template <>
inline constexpr int binary_format<double>::smallest_power_of_ten()
{
    return -342;
}
template <>
inline constexpr int binary_format<float>::smallest_power_of_ten()
{
    return -65;
}

template <> 
inline constexpr std::size_t binary_format<double>::max_digits()
{
    return 769;
}
template <> inline constexpr std::size_t binary_format<float>::max_digits()
{
    return 114;
}

template <> 
inline constexpr binary_format<float>::equiv_uint binary_format<float>::exponent_mask() 
{
    return 0x7F800000;
}
template <> 
inline constexpr binary_format<double>::equiv_uint binary_format<double>::exponent_mask() 
{
    return 0x7FF0000000000000;
}

template <> 
inline constexpr binary_format<float>::equiv_uint binary_format<float>::mantissa_mask()
{
    return 0x007FFFFF;
}
template <> 
inline constexpr binary_format<double>::equiv_uint binary_format<double>::mantissa_mask()
{
    return 0x000FFFFFFFFFFFFF;
}

template <> 
inline constexpr binary_format<float>::equiv_uint binary_format<float>::hidden_bit_mask()
{
    return 0x00800000;
}
template <> 
inline constexpr binary_format<double>::equiv_uint binary_format<double>::hidden_bit_mask()
{
    return 0x0010000000000000;
}

template <typename T>
BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR void to_float(bool negative, adjusted_mantissa am, T &value)
{
    using uint = typename binary_format<T>::equiv_uint;
    uint word = static_cast<uint>(am.mantissa);
    word |= static_cast<uint>(am.power2) << binary_format<T>::mantissa_explicit_bits();
    word |= static_cast<uint>(negative) << binary_format<T>::sign_index();

    #ifdef BOOST_CHARCONV_HAS_STD_BITCAST
    // std::bit_cast is constexpr whereas the boost version is not
    value = std::bit_cast<T>(word);
    
    #else
    
    value = boost::core::bit_cast<T>(word);
    
    #endif
}

template <typename UC>
static constexpr std::uint64_t int_cmp_zeros()
{
    static_assert((sizeof(UC) == 1) || (sizeof(UC) == 2) || (sizeof(UC) == 4), "Unsupported character size");
    return (sizeof(UC) == 1) ? 0x3030303030303030 : (sizeof(UC) == 2) ? 
        (static_cast<uint64_t>(static_cast<UC>('0')) << 48 | 
         static_cast<uint64_t>(static_cast<UC>('0')) << 32 | 
         static_cast<uint64_t>(static_cast<UC>('0')) << 16 | static_cast<UC>('0')) : 
        (static_cast<uint64_t>(static_cast<UC>('0')) << 32 | static_cast<UC>('0'));
}

// Helper C++11 constexpr generic implementation of leading_zeroes
BOOST_FORCEINLINE constexpr
int leading_zeroes_generic(std::uint64_t input_num, int last_bit = 0) {
    return (
        ((input_num & UINT64_C(0xffffffff00000000)) && (input_num >>= 32, last_bit |= 32)),
        ((input_num & UINT64_C(        0xffff0000)) && (input_num >>= 16, last_bit |= 16)),
        ((input_num & UINT64_C(            0xff00)) && (input_num >>=  8, last_bit |=  8)),
        ((input_num & UINT64_C(              0xf0)) && (input_num >>=  4, last_bit |=  4)),
        ((input_num & UINT64_C(               0xc)) && (input_num >>=  2, last_bit |=  2)),
        ((input_num & UINT64_C(               0x2)) && (input_num >>=  1, last_bit |=  1)),
        63 - last_bit
    );
}

// Result might be undefined when input_num is zero
BOOST_FORCEINLINE BOOST_CHARCONV_CXX20_CONSTEXPR
int leading_zeroes(std::uint64_t input_num)
{
    BOOST_CHARCONV_ASSERT(input_num > 0);

    if (BOOST_CHARCONV_IS_CONSTANT_EVALUATED(input_num)) 
    {
        return leading_zeroes_generic(input_num);
    }

    #ifdef BOOST_MSVC
    #  if defined(_M_X64) || defined(_M_ARM64)
        unsigned long leading_zero = 0;
        // Search the mask data from most significant bit (MSB)
        // to least significant bit (LSB) for a set bit (1).
        _BitScanReverse64(&leading_zero, input_num);
        return static_cast<int>(63 - leading_zero);
    
    #  else
    
        return leading_zeroes_generic(input_num);
    
    #  endif
    
    #else

    return __builtin_clzll(input_num);
    
    #endif
}

template <typename UC>
static constexpr int int_cmp_len()
{
    return sizeof(std::uint64_t) / sizeof(UC);
}

template<typename UC>
static constexpr const UC* str_const_nan()
{
    return nullptr;
}

template<>
constexpr const char* str_const_nan<char>()
{
    return "nan";
}
template<>
constexpr const wchar_t* str_const_nan<wchar_t>()
{
    return L"nan";
}
template<>
constexpr const char16_t* str_const_nan<char16_t>()
{
    return u"nan";
}
template<>
constexpr const char32_t* str_const_nan<char32_t>()
{
    return U"nan";
}
template<typename UC>
static constexpr const UC* str_const_inf()
{
    return nullptr;
}
template<>
constexpr const char* str_const_inf<char>()
{
    return "infinity";
}
template<>
constexpr const wchar_t* str_const_inf<wchar_t>()
{
    return L"infinity";
}
template<>
constexpr const char16_t* str_const_inf<char16_t>()
{
    return u"infinity";
}
template<>
constexpr const char32_t* str_const_inf<char32_t>()
{
    return U"infinity";
}

}}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_FAST_FLOAT_FLOAT_COMMON_HPP
