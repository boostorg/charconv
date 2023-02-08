// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/detail/splitmix64.hpp>
#include <iostream>
#include <cstdint>
#include <cfloat>
#include <cmath>

int const N = 1024;

static boost::detail::splitmix64 rng;

// integral types

template<class T> void test_roundtrip( T value, int base )
{
    char buffer[ 256 ] = {};

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ) - 1, value, base );

    BOOST_TEST_EQ( r.ec, 0 );

    T v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2, base );

    BOOST_TEST_EQ( r2.ec, 0 ) && BOOST_TEST_EQ( v2, value );
}

template<class T> void test_roundtrip_int8( int base )
{
    for( int i = -256; i <= 255; ++i )
    {
        test_roundtrip( static_cast<T>( i ), base );
    }
}

template<class T> void test_roundtrip_uint8( int base )
{
    for( int i = 0; i <= 256; ++i )
    {
        test_roundtrip( static_cast<T>( i ), base );
    }
}

template<class T> void test_roundtrip_int16( int base )
{
    test_roundtrip_int8<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::int16_t w = static_cast<std::uint16_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_uint16( int base )
{
    test_roundtrip_uint8<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::uint16_t w = static_cast<std::uint16_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_int32( int base )
{
    test_roundtrip_int16<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::int32_t w = static_cast<std::uint32_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_uint32( int base )
{
    test_roundtrip_uint16<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::uint32_t w = static_cast<std::uint32_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_int64( int base )
{
    test_roundtrip_int32<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::int64_t w = static_cast<std::uint64_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_uint64( int base )
{
    test_roundtrip_uint32<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::uint64_t w = static_cast<std::uint64_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

// floating point types

template<class T> void test_roundtrip( T value )
{
    char buffer[ 256 ] = {};

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ) - 1, value );

    BOOST_TEST_EQ( r.ec, 0 );

    T v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2 );

    if( BOOST_TEST_EQ( r2.ec, 0 ) && BOOST_TEST_EQ( v2, value ) )
    {
    }
    else
    {
        std::cerr << "... test failure for value=" << value << "; buffer='" << std::string( buffer, r.ptr ) << "'" << std::endl;
    }
}

//

int main()
{
    // integral types

    for( int base = 2; base <= 36; ++base )
    {
        test_roundtrip_int8<std::int8_t>( base );
        test_roundtrip_uint8<std::uint8_t>( base );

        test_roundtrip_int16<std::int16_t>( base );
        test_roundtrip_uint16<std::uint16_t>( base );

        test_roundtrip_int32<std::int32_t>( base );
        test_roundtrip_uint32<std::uint32_t>( base );

        test_roundtrip_int64<std::int64_t>( base );
        test_roundtrip_uint64<std::uint64_t>( base );
    }

    // float

    double const q = std::pow( 1.0, -64 );

    {
        for( int i = 0; i < N; ++i )
        {
            float w1 = static_cast<float>( rng() / q );
            test_roundtrip( w1 );

            float w2 = FLT_MAX / static_cast<float>( rng() );
            test_roundtrip( w2 );

            float w3 = FLT_MIN * static_cast<float>( rng() );
            test_roundtrip( w3 );
        }
    }

    // double

    {
        for( int i = 0; i < N; ++i )
        {
            double w1 = rng() / q;
            test_roundtrip( w1 );

            double w2 = DBL_MAX / rng();
            test_roundtrip( w2 );

            double w3 = DBL_MIN * rng();
            test_roundtrip( w3 );
        }
    }

    return boost::report_errors();
}
