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

constexpr int N = 1024;
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

template <typename T>
void test_roundtrip( T value )
{
    char buffer[ 256 ];

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), value );

    BOOST_TEST( r.ec == std::errc() );

    T v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2 );

    if( BOOST_TEST( r2.ec == std::errc() ) && BOOST_TEST( v2 == value ) )
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

const char* fmt_from_type (__float128)
{
    return "%Qg";
}

const char* fmt_from_type_fixed (__float128)
{
    return "%.0f";
}

const char* fmt_from_type_scientific (__float128)
{
    return "%.35Qe";
}

const char* fmt_from_type_hex (__float128)
{
    return "%Qa";
}

inline int print(__float128 value, char* buffer, size_t buffer_size, const char* fmt)
{
    return quadmath_snprintf(buffer, buffer_size, fmt, value);
}

#ifdef BOOST_CHARCONV_HAS_STDFLOAT128
const char* fmt_from_type (std::float128_t)
{
    return "%F128g";
}

const char* fmt_from_type_fixed (std::float128_t)
{
    return "%.0F128f";
}

const char* fmt_from_type_scientific (std::float128_t)
{
    return "%.35F128e";
}

const char* fmt_from_type_hex (std::float128_t)
{
    return "%F128a";
}

inline int print(std::float128_t value, char* buffer, size_t buffer_size, const char* fmt)
{
    return std::snprintf(buffer, buffer_size, fmt, value);
}
#endif

template <typename T>
void test_sprintf_float( T value, boost::charconv::chars_format fmt = boost::charconv::chars_format::scientific )
{
    char buffer [ 256 ] {};

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

    #ifndef __i686__
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
    #endif

    // Same issues that arise in to_chars_snprintf.cpp so abort if in range
    //
    //     Value: 3.350549627872214798203501062446534e-4913
    //  To chars: 3.350549627872214798203501062446534e-4913
    //  Snprintf: 3.35055e-4913
    //
    //     Value: 6.8220421318020332664117517756596e+4913
    //  To chars: 6.8220421318020332664117517756596e+4913
    //  Snprintf: 6.82204e+4913
    if ((value > 1e16Q && value < 1e20Q) ||
        (value > 1e4912Q || value < 1e-4912Q))
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
    test_roundtrip( std::numeric_limits<T>::min() );
    test_roundtrip( -std::numeric_limits<T>::min() );
    test_roundtrip( std::numeric_limits<T>::max() );
    test_roundtrip( -std::numeric_limits<T>::max() );
}

template <>
void test_roundtrip_bv<__float128>()
{
    test_roundtrip( FLT128_MIN );
    test_roundtrip( -FLT128_MIN );
    test_roundtrip( FLT128_MAX );
    test_roundtrip( -FLT128_MAX );
}

int main()
{
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
    #endif

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
