// Copyright 2022, 2023 Peter Dimov
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

char const* fmt_from_type( double )
{
    return "%.17g";
}

char const* fmt_from_type_scientific( double )
{
    return "%.17e";
}

char const* fmt_from_type_fixed( double )
{
    return "%.0f";
}

template<class T> void test_sprintf( T value )
{
    char buffer[ 256 ];

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), value );

    BOOST_TEST_EQ( r.ec, 0 );

    char buffer2[ 256 ];
    std::snprintf( buffer2, sizeof( buffer2 ), fmt_from_type( value ), value );

    BOOST_TEST_EQ( std::string( buffer, r.ptr ), std::string( buffer2 ) );
}

template<class T> void test_sprintf_float( T value, boost::charconv::chars_format fmt )
{
    char buffer[ 256 ];

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), value, fmt );

    BOOST_TEST_EQ( r.ec, 0 );

    char buffer2[ 256 ];

    constexpr T max_value = static_cast<T>((std::numeric_limits<std::uint64_t>::max)());
    constexpr T min_value = 1.0 / max_value;

    if (fmt == boost::charconv::chars_format::general)
    {
        // See https://godbolt.org/z/dd33nM6ax
        if (value < max_value && value > min_value)
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
        std::stringstream ss;
        ss << std::hexfloat << value;
        std::string hex_value = ss.str();
        hex_value = hex_value.substr(2); // Remove the 0x
        std::memcpy(buffer2, hex_value.c_str(), sizeof(buffer2));
    }
    else if (fmt == boost::charconv::chars_format::fixed)
    {
        if (value < max_value && value > min_value)
        {
            std::snprintf( buffer2, sizeof( buffer2 ), fmt_from_type_fixed( value ), value );
        }
        else
        {
            return;
        }
    }

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
    test_sprintf( std::numeric_limits<T>::min() );
    test_sprintf( -std::numeric_limits<T>::min() );
    test_sprintf( std::numeric_limits<T>::max() );
    test_sprintf( +std::numeric_limits<T>::max() );
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
            test_sprintf( w0 );

            float w1 = static_cast<float>( rng() * q ); // 0.0 .. 1.0
            test_sprintf( w1 );

            float w2 = FLT_MAX / static_cast<float>( rng() ); // large values
            test_sprintf( w2 );

            float w3 = FLT_MIN * static_cast<float>( rng() ); // small values
            test_sprintf( w3 );
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
            // test_sprintf_float( w0, boost::charconv::chars_format::hex );
            test_sprintf_float( w0, boost::charconv::chars_format::fixed );

            double w1 = rng() * q; // 0.0 .. 1.0
            test_sprintf_float( w1, boost::charconv::chars_format::general );
            test_sprintf_float( w1, boost::charconv::chars_format::scientific );
            // test_sprintf_float( w1, boost::charconv::chars_format::hex );
            test_sprintf_float( w1, boost::charconv::chars_format::fixed );

            double w2 = DBL_MAX / rng(); // large values
            test_sprintf_float( w2, boost::charconv::chars_format::general );
            test_sprintf_float( w2, boost::charconv::chars_format::scientific );
            // test_sprintf_float( w2, boost::charconv::chars_format::hex );
            test_sprintf_float( w2, boost::charconv::chars_format::fixed );

            double w3 = DBL_MIN * rng(); // small values
            test_sprintf_float( w3, boost::charconv::chars_format::general );
            test_sprintf_float( w3, boost::charconv::chars_format::scientific );
            // test_sprintf_float( w3, boost::charconv::chars_format::hex );
            test_sprintf_float( w3, boost::charconv::chars_format::fixed );
        }

        test_sprintf_bv_fp<double>();
    }

    return boost::report_errors();
}
