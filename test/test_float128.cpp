// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/detail/config.hpp>

#ifdef BOOST_CHARCONV_HAS_FLOAT128

#include <ostream>

#ifdef BOOST_CHARCONV_HAS_STDFLOAT128
#include <charconv>

std::ostream& operator<<( std::ostream& os, __float128 v )
{
    char buffer[ 256 ] {};
    std::to_chars(buffer, buffer + sizeof(buffer), static_cast<std::float128_t>(v));
    os << buffer;
    return os;
}

std::ostream& operator<<( std::ostream& os, std::float128_t v)
{
    char buffer [ 256 ] {};
    std::to_chars(buffer, buffer + sizeof(buffer), v);
    os << buffer;
    return os;
}

#else

std::ostream& operator<<( std::ostream& os, __float128 v )
{
    char buffer[ 256 ] {};
    quadmath_snprintf(buffer, sizeof(buffer), "%Qg", v);
    os << buffer;
    return os;
}

#endif // BOOST_CHARCONV_HAS_STDFLOAT128

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/detail/splitmix64.hpp>
#include <limits>
#include <iostream>
#include <iomanip>
#include <string>
#include <random>

constexpr int N = 10;
static boost::detail::splitmix64 rng;

template <typename T>
void test_signaling_nan()
{
    BOOST_IF_CONSTEXPR (std::is_same<T, __float128>::value)
    {
        #if BOOST_CHARCONV_HAS_BUILTIN(__builtin_nansq)
        BOOST_TEST(boost::charconv::detail::issignaling(__builtin_nansq("")));
        BOOST_TEST(boost::charconv::detail::issignaling(-__builtin_nansq("")));
        #endif
    }
    else
    {
        #ifdef BOOST_CHARCONV_HAS_STDFLOAT128
        BOOST_TEST(boost::charconv::detail::issignaling(std::numeric_limits<T>::signaling_NaN()));
        BOOST_TEST(boost::charconv::detail::issignaling(-std::numeric_limits<T>::signaling_NaN()));
        #endif
    }

    BOOST_TEST(!(boost::charconv::detail::issignaling)(std::numeric_limits<T>::quiet_NaN()));
    BOOST_TEST(!(boost::charconv::detail::issignaling)(std::numeric_limits<T>::infinity()));
    BOOST_TEST(!(boost::charconv::detail::issignaling)(-std::numeric_limits<T>::quiet_NaN()));
    BOOST_TEST(!(boost::charconv::detail::issignaling)(-std::numeric_limits<T>::infinity()));
}

// Derivative of
// https://stackoverflow.com/questions/62074229/float-distance-for-80-bit-long-double
/*  Return the signed distance from 0 to x, measuring distance as one unit per
    number representable in FPType.  x must be a finite number.
*/

#if defined(__GNUC__) && (__GNUC__ >= 5)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wattributes"
#endif

template<typename FPType>
#if !defined(BOOST_MSVC) && !(defined(__clang__) && (__clang_major__ == 3) && (__clang_minor__ < 7))
__attribute__((no_sanitize("undefined")))
#endif
int64_t ToOrdinal(FPType x)
{
    static constexpr int
            Radix             = 2,
            SignificandDigits = FLT128_MANT_DIG,
            MinimumExponent   = FLT128_MIN_EXP;

    //  Number of normal representable numbers for each exponent.
    static const auto
            NumbersPerExponent = static_cast<uint64_t>(scalbn(Radix-1, SignificandDigits-1));

    if (x == 0)
        return 0;

    //  Separate the sign.
    int sign = signbitq(x) ? -1 : +1;
    x = fabsq(x);

    //  Separate the significand and exponent.
    int exponent = ilogbq(x)+1;
    FPType fraction = scalbnq(x, -exponent);

    if (exponent < MinimumExponent)
    {
        //  For subnormal x, adjust to its subnormal representation.
        fraction = scalbnq(fraction, exponent - MinimumExponent);
        exponent = MinimumExponent;
    }

    /*  Start with the number of representable numbers in preceding normal
        exponent ranges.
    */
    int64_t count = (exponent - MinimumExponent) * NumbersPerExponent;

    /*  For subnormal numbers, fraction * radix ** SignificandDigits is the
        number of representable numbers from 0 to x.  For normal numbers,
        (fraction-1) * radix ** SignificandDigits is the number of
        representable numbers from the start of x's exponent range to x, and
        1 * radix ** SignificandDigits is the number of representable subnormal
        numbers (which we have not added into count yet).  So, in either case,
        adding fraction * radix ** SignificandDigits is the desired amount to
        add to count.
    */
    count += (int64_t)scalbnq(fraction, SignificandDigits);

    return sign * count;
}

#if defined(__GNUC__) && (__GNUC__ >= 5)
# pragma GCC diagnostic pop
#endif

/*  Return the number of representable numbers from x to y, including one
    endpoint.
*/
template<typename FPType> int64_t Distance(FPType y, FPType x)
{
    return ToOrdinal(y) - ToOrdinal(x);
}

template <typename T>
void test_roundtrip( T value )
{
    char buffer[ 256 ];

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), value );

    BOOST_TEST( r.ec == std::errc() );

    T v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2 );

    if( BOOST_TEST( r2.ec == std::errc() ) && BOOST_TEST( Distance(v2, value) <= 1 ) )
    {
    }
    else
    {
        std::cerr << std::setprecision(35)
                  << "     Value: " << value
                  << "\n  To chars: " << std::string( buffer, r.ptr )
                  << "\nFrom chars: " << v2 << std::endl;
    }
}

template <typename T>
const char* fmt_from_type (T)
{
    return "%Qg";
}

template <typename T>
const char* fmt_from_type_fixed (T)
{
    return "%.0f";
}

template <typename T>
const char* fmt_from_type_scientific (T)
{
    return "%.35Qe";
}

template <typename T>
const char* fmt_from_type_hex (T)
{
    return "%Qa";
}

inline int print(__float128 value, char* buffer, size_t buffer_size, const char* fmt)
{
    return quadmath_snprintf(buffer, buffer_size, fmt, value);
}

#ifdef BOOST_CHARCONV_HAS_STDFLOAT128
// Has no overload of strtod/sprintf etc so cast to __float128
// See: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1467r9.html#library
inline int print(std::float128_t value, char* buffer, size_t buffer_size, const char* fmt)
{
    return print(static_cast<__float128>(value), buffer, buffer_size, fmt);
}

#endif

template <typename T>
void test_sprintf_float( T value, boost::charconv::chars_format fmt = boost::charconv::chars_format::scientific )
{
    char buffer [ 256 ] {};

    if (fmt == boost::charconv::chars_format::fixed && (value > 1e100L || value < 1e-100L))
    {
        // Avoid failures from overflow
        return;
    }

    const auto r = boost::charconv::to_chars( buffer, buffer + sizeof(buffer), value, fmt );
    BOOST_TEST( r.ec == std::errc() );

    char buffer2 [ 256 ] {};

    const char* sprintf_fmt;
    const char* error_format;
    switch (fmt)
    {
        case boost::charconv::chars_format::general:
            sprintf_fmt = fmt_from_type(value);
            error_format = "General";
            break;
        case boost::charconv::chars_format::scientific:
            sprintf_fmt = fmt_from_type_scientific(value);
            error_format = "Scientific";
            break;
        case boost::charconv::chars_format::fixed:
            sprintf_fmt = fmt_from_type_fixed(value);
            error_format = "Fixed";
            break;
        case boost::charconv::chars_format::hex:
            sprintf_fmt = fmt_from_type_hex(value);
            error_format = "Hex";
            break;
    }

    print( value, buffer2, sizeof(buffer2), sprintf_fmt );

    // Remove trailing zeros from printf (if applicable)
    std::string printf_string {buffer2};
    if (fmt == boost::charconv::chars_format::scientific)
    {
        std::size_t found_trailing_0 = printf_string.find_first_of('e');
        if (found_trailing_0 != std::string::npos)
        {
            --found_trailing_0;
            while (printf_string[found_trailing_0] == '0')
            {
                printf_string.erase(found_trailing_0, 1);
                --found_trailing_0;
            }
        }
    }

    // Same issues that arise in to_chars_snprintf.cpp so abort if in range
    //
    //     Value: 3.350549627872214798203501062446534e-4913
    //  To chars: 3.350549627872214798203501062446534e-4913
    //  Snprintf: 3.35055e-4913
    //
    //     Value: 6.8220421318020332664117517756596e+4913
    //  To chars: 6.8220421318020332664117517756596e+4913
    //  Snprintf: 6.82204e+4913
    //
    //     Value: 1.0600979293241972185e-109
    //  To chars: 1.0600979293241972185e-109
    //  Snprintf: 1.0601e-109
    //
    if ((value > static_cast<T>(1e16Q) && value < static_cast<T>(1e20Q)) ||
        (value > static_cast<T>(1e4912Q) || value < static_cast<T>(1e-4912Q)) ||
        (value > static_cast<T>(1e-115Q) && value < static_cast<T>(2e-109Q)))
    {
        return;
    }

    if( BOOST_TEST_EQ( std::string( buffer, r.ptr ), printf_string ) )
    {
    }
    else
    {
        std::cerr << std::setprecision(35)
                  << "     Value: " << value
                  << "\n  To chars: " << std::string( buffer, r.ptr )
                  << "\n  Snprintf: " << printf_string
                  << "\n    Format: " << error_format << std::endl;
    }
}

template <typename T>
void test_roundtrip_bv()
{
    test_roundtrip( static_cast<T>(FLT128_MIN) );
    test_roundtrip( static_cast<T>(-FLT128_MIN) );
    test_roundtrip( static_cast<T>(FLT128_MAX) );
    test_roundtrip( static_cast<T>(-FLT128_MAX) );
}

#ifdef BOOST_CHARCONV_HAS_STDFLOAT128
template <typename T>
void test_spot(T val, boost::charconv::chars_format fmt = boost::charconv::chars_format::general, int precision = -1)
{
    if (fmt == boost::charconv::chars_format::fixed && (val > 1e100 || val < 1e-100))
    {
        // Avoid failres from overflow
        return;
    }

    std::chars_format stl_fmt;
    switch (fmt)
    {
        case boost::charconv::chars_format::general:
            stl_fmt = std::chars_format::general;
            break;
        case boost::charconv::chars_format::fixed:
            stl_fmt = std::chars_format::fixed;
            break;
        case boost::charconv::chars_format::scientific:
            stl_fmt = std::chars_format::scientific;
            break;
        case boost::charconv::chars_format::hex:
            stl_fmt = std::chars_format::hex;
            break;
        default:
            BOOST_UNREACHABLE_RETURN(fmt);
            break;
    }

    char buffer_boost[256];
    char buffer_stl[256];

    boost::charconv::to_chars_result r_boost;
    std::to_chars_result r_stl;

    if (precision == -1)
    {
        r_boost = boost::charconv::to_chars(buffer_boost, buffer_boost + sizeof(buffer_boost), val, fmt);
        r_stl = std::to_chars(buffer_stl, buffer_stl + sizeof(buffer_stl), val, stl_fmt);
    }
    else
    {
        r_boost = boost::charconv::to_chars(buffer_boost, buffer_boost + sizeof(buffer_boost), val, fmt, precision);
        r_stl = std::to_chars(buffer_stl, buffer_stl + sizeof(buffer_stl), val, stl_fmt, precision);
    }

    BOOST_TEST(r_boost.ec == std::errc());
    if (r_stl.ec != std::errc())
    {
        // STL failed
        return;
    }

    const std::ptrdiff_t diff_boost = r_boost.ptr - buffer_boost;
    const std::ptrdiff_t diff_stl = r_stl.ptr - buffer_stl;
    const auto boost_str = std::string(buffer_boost, r_boost.ptr);
    const auto stl_str = std::string(buffer_stl, r_stl.ptr);

    if (!(BOOST_TEST_CSTR_EQ(boost_str.c_str(), stl_str.c_str()) && BOOST_TEST_EQ(diff_boost, diff_stl)))
    {
        std::cerr << std::setprecision(35)
                  << "Value: " << val
                  << "\nBoost: " << boost_str.c_str()
                  << "\n  STL: " << stl_str.c_str() << std::endl;
    }
}

template <typename T>
void random_test(boost::charconv::chars_format fmt = boost::charconv::chars_format::general)
{
    std::mt19937_64 gen(42);
    std::uniform_real_distribution<T> dist(0, FLT128_MAX);

    for (int i = 0; i < N; ++i)
    {
        test_spot<T>(dist(gen), fmt);
    }
}
#endif

int main()
{
    #if BOOST_CHARCONV_LDBL_BITS == 128
    test_signaling_nan<long double>();

    // 128-bit long double
    {
        const long double q = powq( 1.0L, -128.0L );

        for( int i = 0; i < N; ++i )
        {
            long double w0 = static_cast<long double>( rng() ); // 0 .. 2^128
            test_roundtrip( w0 );
            test_sprintf_float( w0, boost::charconv::chars_format::general );
            test_sprintf_float( w0, boost::charconv::chars_format::scientific );
            test_sprintf_float( w0, boost::charconv::chars_format::fixed );
            test_sprintf_float( w0, boost::charconv::chars_format::hex );

            long double w1 = static_cast<long double>( rng() * q ); // 0.0 .. 1.0
            test_roundtrip( w1 );
            test_sprintf_float( w1, boost::charconv::chars_format::general );
            test_sprintf_float( w1, boost::charconv::chars_format::scientific );
            test_sprintf_float( w1, boost::charconv::chars_format::fixed );
            test_sprintf_float( w1, boost::charconv::chars_format::hex );

            long double w2 = FLT128_MAX / static_cast<long double>( rng() ); // large values
            test_roundtrip( w2 );
            test_sprintf_float( w2, boost::charconv::chars_format::general );
            test_sprintf_float( w2, boost::charconv::chars_format::scientific );
            test_sprintf_float( w2, boost::charconv::chars_format::fixed );
            test_sprintf_float( w2, boost::charconv::chars_format::hex );

            long double w3 = FLT128_MIN * static_cast<long double>( rng() ); // small values
            test_roundtrip( w3 );
            test_sprintf_float( w3, boost::charconv::chars_format::general );
            test_sprintf_float( w3, boost::charconv::chars_format::scientific );
            test_sprintf_float( w3, boost::charconv::chars_format::fixed );
            test_sprintf_float( w3, boost::charconv::chars_format::hex );
        }

        test_roundtrip_bv<__float128>();
    }
    #endif

    test_signaling_nan<__float128>();

    // __float128
    {
        const __float128 q = powq( 1.0Q, -128.0Q );

        for( int i = 0; i < N; ++i )
        {
            __float128 w0 = static_cast<__float128>( rng() ); // 0 .. 2^128
            test_roundtrip( w0 );
            test_sprintf_float( w0, boost::charconv::chars_format::general );
            test_sprintf_float( w0, boost::charconv::chars_format::scientific );
            test_sprintf_float( w0, boost::charconv::chars_format::fixed );
            test_sprintf_float( w0, boost::charconv::chars_format::hex );

            __float128 w1 = static_cast<__float128>( rng() * q ); // 0.0 .. 1.0
            test_roundtrip( w1 );
            test_sprintf_float( w1, boost::charconv::chars_format::general );
            test_sprintf_float( w1, boost::charconv::chars_format::scientific );
            test_sprintf_float( w1, boost::charconv::chars_format::fixed );
            test_sprintf_float( w1, boost::charconv::chars_format::hex );

            __float128 w2 = FLT128_MAX / static_cast<__float128>( rng() ); // large values
            test_roundtrip( w2 );
            test_sprintf_float( w2, boost::charconv::chars_format::general );
            test_sprintf_float( w2, boost::charconv::chars_format::scientific );
            test_sprintf_float( w2, boost::charconv::chars_format::fixed );
            test_sprintf_float( w2, boost::charconv::chars_format::hex );

            __float128 w3 = FLT128_MIN * static_cast<__float128>( rng() ); // small values
            test_roundtrip( w3 );
            test_sprintf_float( w3, boost::charconv::chars_format::general );
            test_sprintf_float( w3, boost::charconv::chars_format::scientific );
            test_sprintf_float( w3, boost::charconv::chars_format::fixed );
            test_sprintf_float( w3, boost::charconv::chars_format::hex );
        }

        test_roundtrip_bv<__float128>();
    }

    #ifdef BOOST_CHARCONV_HAS_STDFLOAT128
    test_signaling_nan<std::float128_t>();

    // std::float128_t
    {
        const std::float128_t q = 1.0e-128F128;

        for( int i = 0; i < N; ++i )
        {
            std::float128_t w0 = static_cast<std::float128_t>( rng() ); // 0 .. 2^128
            test_roundtrip( w0 );
            test_sprintf_float( w0, boost::charconv::chars_format::general );
            test_sprintf_float( w0, boost::charconv::chars_format::scientific );
            test_sprintf_float( w0, boost::charconv::chars_format::fixed );
            test_sprintf_float( w0, boost::charconv::chars_format::hex );

            test_spot( w0, boost::charconv::chars_format::general );
            test_spot( w0, boost::charconv::chars_format::scientific );
            test_spot( w0, boost::charconv::chars_format::fixed );
            test_spot( w0, boost::charconv::chars_format::hex );

            test_spot( w0, boost::charconv::chars_format::general, 6 );
            test_spot( w0, boost::charconv::chars_format::scientific, 8 );

            std::float128_t w1 = static_cast<std::float128_t>( rng() * q ); // 0.0 .. 1.0
            test_roundtrip( w1 );
            test_sprintf_float( w1, boost::charconv::chars_format::general );
            test_sprintf_float( w1, boost::charconv::chars_format::scientific );
            test_sprintf_float( w1, boost::charconv::chars_format::fixed );
            test_sprintf_float( w1, boost::charconv::chars_format::hex );

            test_spot( w1, boost::charconv::chars_format::general );
            test_spot( w1, boost::charconv::chars_format::scientific );
            test_spot( w1, boost::charconv::chars_format::fixed );
            test_spot( w1, boost::charconv::chars_format::hex );

            test_spot( w1, boost::charconv::chars_format::general, 6 );
            test_spot( w1, boost::charconv::chars_format::scientific, 8 );

            // std::numeric_limits<std::float128_t> was not specialized until GCC-14
            // same with __float128

            std::float128_t w2 = static_cast<std::float128_t>(FLT128_MAX) / static_cast<std::float128_t>( rng() ); // large values
            test_roundtrip( w2 );
            test_sprintf_float( w2, boost::charconv::chars_format::general );
            test_sprintf_float( w2, boost::charconv::chars_format::scientific );
            test_sprintf_float( w2, boost::charconv::chars_format::fixed );
            test_sprintf_float( w2, boost::charconv::chars_format::hex );

            test_spot( w2, boost::charconv::chars_format::general );
            test_spot( w2, boost::charconv::chars_format::scientific );
            test_spot( w2, boost::charconv::chars_format::fixed );
            test_spot( w2, boost::charconv::chars_format::hex );

            test_spot( w2, boost::charconv::chars_format::general, 6 );
            test_spot( w2, boost::charconv::chars_format::scientific, 8 );

            std::float128_t w3 = static_cast<std::float128_t>(FLT128_MIN) * static_cast<std::float128_t>( rng() ); // small values
            test_roundtrip( w3 );
            test_sprintf_float( w3, boost::charconv::chars_format::general );
            test_sprintf_float( w3, boost::charconv::chars_format::scientific );
            test_sprintf_float( w3, boost::charconv::chars_format::fixed );
            test_sprintf_float( w3, boost::charconv::chars_format::hex );

            test_spot( w3, boost::charconv::chars_format::general );
            test_spot( w3, boost::charconv::chars_format::scientific );
            test_spot( w3, boost::charconv::chars_format::fixed );
            test_spot( w3, boost::charconv::chars_format::hex );

            test_spot( w3, boost::charconv::chars_format::general, 6 );
            test_spot( w3, boost::charconv::chars_format::scientific, 8 );
        }

        test_roundtrip_bv<std::float128_t>();
    }

    random_test<__float128>();
    random_test<std::float128_t>();
    
    #endif

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
