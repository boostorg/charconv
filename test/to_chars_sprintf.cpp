// Copyright 2022, 2023 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/detail/splitmix64.hpp>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <cstdint>
#include <cfloat>
#include <cmath>

int const N = 1024;

static boost::detail::splitmix64 rng;

//

char const* fmt_from_type( int )
{
    return "%d";
}

char const* fmt_from_type( unsigned )
{
    return "%u";
}

char const* fmt_from_type( long )
{
    return "%ld";
}

char const* fmt_from_type( unsigned long )
{
    return "%lu";
}

char const* fmt_from_type( long long )
{
    return "%lld";
}

char const* fmt_from_type( unsigned long long )
{
    return "%llu";
}

char const* fmt_from_type( float )
{
    return "%.9g";
}

char const* fmt_from_type_scientific( float )
{
    return "%.9e";
}

char const* fmt_from_type_fixed( float )
{
    return "%.0f";
}

char const* fmt_from_type( double )
{
    return "%g";
}

char const* fmt_from_type_scientific( double )
{
    return "%.17e";
}

char const* fmt_from_type_fixed( double )
{
    return "%.0f";
}

#if BOOST_CHARCONV_LDBL_BITS == 64 || defined(BOOST_MSVC)
char const* fmt_from_type( long double )
{
    return "%g";
}

char const* fmt_from_type_scientific( long double )
{
    return "%.17e";
}

char const* fmt_from_type_fixed( long double )
{
    return "%.0f";
}
#endif

template<class T> void test_sprintf( T value )
{
    char buffer[ 256 ];

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), value );

    BOOST_TEST_EQ( r.ec, 0 );

    char buffer2[ 256 ];
    std::snprintf( buffer2, sizeof( buffer2 ), fmt_from_type( value ), value );

    BOOST_TEST_EQ( std::string( buffer, r.ptr ), std::string( buffer2 ) );
}

#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4127) // Conditional expression is constant (e.g. BOOST_IF_CONSTEXPR statements)
#endif

template<class T> void test_sprintf_float( T value, boost::charconv::chars_format fmt )
{
    char buffer[ 256 ];

        
    boost::charconv::to_chars_result r;
    if (fmt != boost::charconv::chars_format::scientific)
    {
        r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), value, fmt );
    }
    else
    {
        // Sprintf uses 9 / 17 digits of precision
        r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), value, fmt, std::numeric_limits<T>::max_digits10);
    }

    BOOST_TEST_EQ( r.ec, 0 );

    char buffer2[ 256 ];

    T max_value;
    BOOST_IF_CONSTEXPR (std::is_same<T, float>::value)
    {
        max_value = static_cast<T>((std::numeric_limits<std::uint32_t>::max)());
    }
    else BOOST_IF_CONSTEXPR(std::is_same<T, double>::value
                            #if BOOST_CHARCONV_LDBL_BITS == 64 || defined(BOOST_MSVC)
                            || std::is_same<T, long double>::value
                            #endif
                           )
    {
        max_value = static_cast<T>(1e16);
    }

    if (fmt == boost::charconv::chars_format::general)
    {
        // See https://godbolt.org/z/dd33nM6ax
        if (value >= 1 && value < max_value)
        {
            std::snprintf( buffer2, sizeof( buffer2 ), fmt_from_type_fixed( value ), value );
        }
        else
        {            
            std::snprintf( buffer2, sizeof( buffer2 ), fmt_from_type( value ), value );
        }
    }
    else if (fmt == boost::charconv::chars_format::scientific)
    {
        std::snprintf( buffer2, sizeof( buffer2 ), fmt_from_type_scientific( value ), value );
    }
    else if (fmt == boost::charconv::chars_format::hex)
    {
        // GCC 4.X does not support std:::hexfloat
        // GCC and Clang on Windows both diverge slightly from what they should be doing
        #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && !defined(_MSC_VER))

        std::stringstream ss;
        ss << std::hexfloat << value;
        std::string hex_value = ss.str();
        hex_value = hex_value.substr(2); // Remove the 0x
        std::memcpy(buffer2, hex_value.c_str(), hex_value.size() + 1);

        #endif
    }
    else if (fmt == boost::charconv::chars_format::fixed)
    {
        if (value >= 1 && value < max_value)
        {
            std::snprintf( buffer2, sizeof( buffer2 ), fmt_from_type_fixed( value ), value );
        }
        else
        {
            return;
        }
    }

    // Remove ranges where sprintf does not perform like it should under general formatting:
    // See to_chars_float_STL_comp.cpp
    //
    //       Value: 3.78882532780079974e+18
    //    To chars: 3.7888253278007997e+18
    //    Snprintf: 3.78883e+18
    // 
    //       Value: 2.25907093342864926e-289
    //    To chars: 2.2590709334286493e-289
    //    Snprintf: 2.25907e-289
    //
    //       Value: 2.98808092305723294e+289
    //    To chars: 2.988080923057233e+289
    //    Snprintf: 2.98808e+289
    
    BOOST_IF_CONSTEXPR(std::is_same<T, double>::value
                       #if BOOST_CHARCONV_LDBL_BITS == 64 || defined(BOOST_MSVC)
                       || std::is_same<T, long double>::value
                       #endif
                       )
    {
        if ( !(((value > 1e16 && value < 1e20) || (value < 1e-288 && value > 0) || (value > 1e288) ) && fmt == boost::charconv::chars_format::general))
        {
            if(!BOOST_TEST_EQ( std::string( buffer, r.ptr ), std::string( buffer2 ) ))
            {
                // Set precision for integer part + decimal digits
                // See: https://en.cppreference.com/w/cpp/io/manip/setprecision
                std::cerr << std::setprecision(std::numeric_limits<T>::max_digits10 + 1)
                        << "   Value: " << value
                        << "\nTo chars: " << std::string( buffer, r.ptr )
                        << "\nSnprintf: " << std::string( buffer2 ) << std::endl;
            }
        }
    }
    else
    {
        if ( !(((value > 1e15 && value < 1e23) || (value < 1e-18 && value > 0) ) && fmt == boost::charconv::chars_format::general))
        {
            if(!BOOST_TEST_EQ( std::string( buffer, r.ptr ), std::string( buffer2 ) ))
            {
                // Set precision for integer part + decimal digits
                // See: https://en.cppreference.com/w/cpp/io/manip/setprecision
                std::cerr << std::setprecision(std::numeric_limits<T>::max_digits10 + 1)
                        << "   Value: " << value
                        << "\nTo chars: " << std::string( buffer, r.ptr )
                        << "\nSnprintf: " << std::string( buffer2 ) << std::endl;
            }
        }        
    }
}

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif

// integral types, random values

template<class T> void test_sprintf_int8()
{
    for( int i = -256; i <= 255; ++i )
    {
        test_sprintf( static_cast<T>( i ) );
    }
}

template<class T> void test_sprintf_uint8()
{
    for( int i = 0; i <= 256; ++i )
    {
        test_sprintf( static_cast<T>( i ) );
    }
}

template<class T> void test_sprintf_int16()
{
    test_sprintf_int8<T>();

    for( int i = 0; i < N; ++i )
    {
        std::int16_t w = static_cast<std::uint16_t>( rng() );
        test_sprintf( static_cast<T>( w ) );
    }
}

template<class T> void test_sprintf_uint16()
{
    test_sprintf_uint8<T>();

    for( int i = 0; i < N; ++i )
    {
        std::uint16_t w = static_cast<std::uint16_t>( rng() );
        test_sprintf( static_cast<T>( w ) );
    }
}

template<class T> void test_sprintf_int32()
{
    test_sprintf_int16<T>();

    for( int i = 0; i < N; ++i )
    {
        std::int32_t w = static_cast<std::uint32_t>( rng() );
        test_sprintf( static_cast<T>( w ) );
    }
}

template<class T> void test_sprintf_uint32()
{
    test_sprintf_uint16<T>();

    for( int i = 0; i < N; ++i )
    {
        std::uint32_t w = static_cast<std::uint32_t>( rng() );
        test_sprintf( static_cast<T>( w ) );
    }
}

template<class T> void test_sprintf_int64()
{
    test_sprintf_int32<T>();

    for( int i = 0; i < N; ++i )
    {
        std::int64_t w = static_cast<std::uint64_t>( rng() );
        test_sprintf( static_cast<T>( w ) );
    }
}

template<class T> void test_sprintf_uint64()
{
    test_sprintf_uint32<T>();

    for( int i = 0; i < N; ++i )
    {
        std::uint64_t w = static_cast<std::uint64_t>( rng() );
        test_sprintf( static_cast<T>( w ) );
    }
}

// integral types, boundary values

template<class T> void test_sprintf_bv()
{
    test_sprintf( std::numeric_limits<T>::min() );
    test_sprintf( std::numeric_limits<T>::max() );
}

// floating point types, boundary values

template<class T> void test_sprintf_bv_fp()
{
    test_sprintf_float( std::numeric_limits<T>::min(), boost::charconv::chars_format::scientific );
    test_sprintf_float( -std::numeric_limits<T>::min(), boost::charconv::chars_format::scientific );
    test_sprintf_float( std::numeric_limits<T>::max(), boost::charconv::chars_format::scientific );
    test_sprintf_float( +std::numeric_limits<T>::max(), boost::charconv::chars_format::scientific );
}

//

int main()
{
    // integral types, random values

    test_sprintf_int8<std::int8_t>();
    test_sprintf_uint8<std::uint8_t>();

    test_sprintf_int16<std::int16_t>();
    test_sprintf_uint16<std::uint16_t>();

    test_sprintf_int32<std::int32_t>();
    test_sprintf_uint32<std::uint32_t>();

    test_sprintf_int64<std::int64_t>();
    test_sprintf_uint64<std::uint64_t>();

    test_sprintf_bv<char>();
    test_sprintf_bv<signed char>();
    test_sprintf_bv<unsigned char>();

    test_sprintf_bv<short>();
    test_sprintf_bv<unsigned short>();

    test_sprintf_bv<int>();
    test_sprintf_bv<unsigned int>();

    test_sprintf_bv<long>();
    test_sprintf_bv<unsigned long>();

    test_sprintf_bv<long long>();
    test_sprintf_bv<unsigned long long>();

    // float

    double const q = std::pow( 1.0, -64 );

    {
        for( int i = 0; i < N; ++i )
        {
            float w0 = static_cast<float>( rng() ); // 0 .. 2^64
            test_sprintf_float( w0, boost::charconv::chars_format::general );
            test_sprintf_float( w0, boost::charconv::chars_format::scientific );
            test_sprintf_float( w0, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w0, boost::charconv::chars_format::hex );
            #endif

            float w1 = static_cast<float>( rng() * q ); // 0.0 .. 1.0
            test_sprintf_float( w1, boost::charconv::chars_format::general );
            test_sprintf_float( w1, boost::charconv::chars_format::scientific );
            test_sprintf_float( w1, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w1, boost::charconv::chars_format::hex );
            #endif


            float w2 = FLT_MAX / static_cast<float>( rng() ); // large values
            test_sprintf_float( w2, boost::charconv::chars_format::general );
            test_sprintf_float( w2, boost::charconv::chars_format::scientific );
            test_sprintf_float( w2, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w2, boost::charconv::chars_format::hex );
            #endif


            float w3 = FLT_MIN * static_cast<float>( rng() ); // small values
            test_sprintf_float( w3, boost::charconv::chars_format::general );
            test_sprintf_float( w3, boost::charconv::chars_format::scientific );
            test_sprintf_float( w3, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w3, boost::charconv::chars_format::hex );
            #endif

        }

        test_sprintf_bv_fp<float>();
    }

    // double

    {
        for( int i = 0; i < N; ++i )
        {
            double w0 = rng() * 1.0; // 0 .. 2^64
            test_sprintf_float( w0, boost::charconv::chars_format::general );
            test_sprintf_float( w0, boost::charconv::chars_format::scientific );
            test_sprintf_float( w0, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w0, boost::charconv::chars_format::hex );
            #endif

            double w1 = rng() * q; // 0.0 .. 1.0
            test_sprintf_float( w1, boost::charconv::chars_format::general );
            test_sprintf_float( w1, boost::charconv::chars_format::scientific );
            test_sprintf_float( w1, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w1, boost::charconv::chars_format::hex );
            #endif

            double w2 = DBL_MAX / rng(); // large values
            test_sprintf_float( w2, boost::charconv::chars_format::general );
            test_sprintf_float( w2, boost::charconv::chars_format::scientific );
            test_sprintf_float( w2, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w2, boost::charconv::chars_format::hex );
            #endif

            double w3 = DBL_MIN * rng(); // small values
            test_sprintf_float( w3, boost::charconv::chars_format::general );
            test_sprintf_float( w3, boost::charconv::chars_format::scientific );
            test_sprintf_float( w3, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w3, boost::charconv::chars_format::hex );
            #endif
        }

        test_sprintf_bv_fp<double>();
    }

    #ifdef BOOST_CHARCONV_FULL_LONG_DOUBLE_TO_CHARS_IMPL
    // long double

    {
        for( int i = 0; i < N; ++i )
        {
            long double w0 = rng() * 1.0; // 0 .. 2^64
            test_sprintf_float( w0, boost::charconv::chars_format::general );
            test_sprintf_float( w0, boost::charconv::chars_format::scientific );
            test_sprintf_float( w0, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w0, boost::charconv::chars_format::hex );
            #endif

            long double w1 = rng() * q; // 0.0 .. 1.0
            test_sprintf_float( w1, boost::charconv::chars_format::general );
            test_sprintf_float( w1, boost::charconv::chars_format::scientific );
            test_sprintf_float( w1, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w1, boost::charconv::chars_format::hex );
            #endif

            long double w2 = DBL_MAX / rng(); // large values
            test_sprintf_float( w2, boost::charconv::chars_format::general );
            test_sprintf_float( w2, boost::charconv::chars_format::scientific );
            test_sprintf_float( w2, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w2, boost::charconv::chars_format::hex );
            #endif

            long double w3 = DBL_MIN * rng(); // small values
            test_sprintf_float( w3, boost::charconv::chars_format::general );
            test_sprintf_float( w3, boost::charconv::chars_format::scientific );
            test_sprintf_float( w3, boost::charconv::chars_format::fixed );
            #if ((defined(__GNUC__) && __GNUC__ > 4) || defined(__clang__)) && !(defined(BOOST_WINDOWS) && (defined(__clang__) || defined(__GNUC__)))
            test_sprintf_float( w3, boost::charconv::chars_format::hex );
            #endif
        }

        test_sprintf_bv_fp<long double>();
    }
    #endif // BOOST_CHARCONV_FULL_LONG_DOUBLE_TO_CHARS_IMPL

    return boost::report_errors();
}
