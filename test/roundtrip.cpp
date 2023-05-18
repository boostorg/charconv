// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/config.hpp>

#ifdef BOOST_HAS_INT128

// We need to define these operator<< overloads before
// including boost/core/lightweight_test.hpp, or they
// won't be visible to BOOST_TEST_EQ

#include <ostream>

static char* mini_to_chars( char (&buffer)[ 64 ], boost::uint128_type v )
{
    char* p = buffer + 64;
    *--p = '\0';

    do
    {
        *--p = "0123456789"[ v % 10 ];
        v /= 10;
    }
    while ( v != 0 );

    return p;
}

std::ostream& operator<<( std::ostream& os, boost::uint128_type v )
{
    char buffer[ 64 ];

    os << mini_to_chars( buffer, v );
    return os;
}

std::ostream& operator<<( std::ostream& os, boost::int128_type v )
{
    char buffer[ 64 ];
    char* p;

    if( v >= 0 )
    {
        p = mini_to_chars( buffer, v );
    }
    else
    {
        p = mini_to_chars( buffer, -(boost::uint128_type)v );
        *--p = '-';
    }

    os << p;
    return os;
}

#endif // #ifdef BOOST_HAS_INT128

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/detail/splitmix64.hpp>
#include <system_error>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cstdint>
#include <cfloat>
#include <cmath>

int const N = 1024;

static boost::detail::splitmix64 rng;

// integral types, random values

#if defined(__GNUC__) && (__GNUC__ == 12)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

template<class T> void test_roundtrip( T value, int base )
{
    char buffer[ 256 ];

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), value, base );

    BOOST_TEST( r.ec == std::errc() );

    T v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2, base );

    if( BOOST_TEST( r2.ec == std::errc() ) && BOOST_TEST_EQ( v2, value ) )
    {
    }
    else
    {
        std::cerr << "... test failure for value=" << value << "; buffer='" << std::string( buffer, r.ptr ) << "'" << std::endl;
    }
}

#if defined(__GNUC__) && (__GNUC__ == 12)
# pragma GCC diagnostic pop
#endif

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

#ifdef BOOST_CHARCONV_HAS_INT128

inline boost::uint128_type concatenate(std::uint64_t word1, std::uint64_t word2)
{
    return static_cast<boost::uint128_type>(word1) << 64 | word2;
}

template<class T> void test_roundtrip_int128( int base )
{
    for( int i = 0; i < N; ++i )
    {
        boost::int128_type w = static_cast<boost::uint128_type>( concatenate(rng(), rng()) );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_uint128( int base )
{
    for( int i = 0; i < N; ++i )
    {
        boost::uint128_type w = static_cast<boost::uint128_type>( concatenate(rng(), rng()) );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

#endif // #ifdef BOOST_CHARCONV_HAS_INT128

// integral types, boundary values

template<class T> void test_roundtrip_bv( int base )
{
    test_roundtrip( std::numeric_limits<T>::min(), base );
    test_roundtrip( std::numeric_limits<T>::max(), base );
}

// floating point types, random values

template<class T> void test_roundtrip( T value )
{
    char buffer[ 256 ];

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), value );

    BOOST_TEST( r.ec == std::errc() );

    T v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2 );

    if( BOOST_TEST( r2.ec == std::errc() ) && BOOST_TEST_EQ( v2, value ) )
    {
    }
    else
    {
        #ifdef BOOST_CHARCONV_DEBUG
        std::cerr << std::setprecision(17)
                  << "     Value: " << value
                  << "\n  To chars: " << std::string( buffer, r.ptr )
                  << "\nFrom chars: " << v2 << std::endl;
        #else
        std::cerr << "... test failure for value=" << value << "; buffer='" << std::string( buffer, r.ptr ) << "'" << std::endl;
        #endif
    }
}

// floating point types, boundary values

template<class T> void test_roundtrip_bv()
{
    test_roundtrip( std::numeric_limits<T>::min() );
    test_roundtrip( -std::numeric_limits<T>::min() );
    test_roundtrip( std::numeric_limits<T>::max() );
    test_roundtrip( +std::numeric_limits<T>::max() );
}

//

int main()
{
    // integral types, random values

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

#ifdef BOOST_CHARCONV_HAS_INT128

        test_roundtrip_int128<boost::int128_type>( base );
        test_roundtrip_uint128<boost::uint128_type>( base );

#endif
    }

    // integral types, boundary values

    for( int base = 2; base <= 36; ++base )
    {
        test_roundtrip_bv<char>( base );
        test_roundtrip_bv<signed char>( base );
        test_roundtrip_bv<unsigned char>( base );

        test_roundtrip_bv<short>( base );
        test_roundtrip_bv<unsigned short>( base );

        test_roundtrip_bv<int>( base );
        test_roundtrip_bv<unsigned int>( base );

        test_roundtrip_bv<long>( base );
        test_roundtrip_bv<unsigned long>( base );

        test_roundtrip_bv<long long>( base );
        test_roundtrip_bv<unsigned long long>( base );

#ifdef BOOST_CHARCONV_HAS_INT128

        test_roundtrip_bv<boost::int128_type>( base );
        test_roundtrip_bv<boost::uint128_type>( base );

#endif
    }

#if !defined(__CYGWIN__) && !defined(__s390x__) && !((defined(__arm__) || defined(__aarch64__))  && !defined(__APPLE__)) && !(defined(__APPLE__) && (__clang_major__ == 12))

    // the stub implementations fail under Cygwin, s390x, linux ARM, and Apple Clang w/Xcode 12.2;
    // re-enable these when we have real ones

    // float

    double const q = std::pow( 1.0, -64 );

    {
        for( int i = 0; i < N; ++i )
        {
            float w0 = static_cast<float>( rng() ); // 0 .. 2^64
            test_roundtrip( w0 );

            float w1 = static_cast<float>( rng() * q ); // 0.0 .. 1.0
            test_roundtrip( w1 );

            float w2 = FLT_MAX / static_cast<float>( rng() ); // large values
            test_roundtrip( w2 );

            float w3 = FLT_MIN * static_cast<float>( rng() ); // small values
            test_roundtrip( w3 );
        }

        test_roundtrip_bv<float>();
    }

    // double

    {
        for( int i = 0; i < N; ++i )
        {
            double w0 = rng() * 1.0; // 0 .. 2^64
            test_roundtrip( w0 );

            double w1 = rng() * q; // 0.0 .. 1.0
            test_roundtrip( w1 );

            double w2 = DBL_MAX / rng(); // large values
            test_roundtrip( w2 );

            double w3 = DBL_MIN * rng(); // small values
            test_roundtrip( w3 );
        }

        test_roundtrip_bv<double>();
    }

    // long double

    {
        long double const ql = std::pow( 1.0L, -64 );

        for( int i = 0; i < N; ++i )
        {
            long double w0 = rng() * 1.0L; // 0 .. 2^64
            test_roundtrip( w0 );

            long double w1 = rng() * ql; // 0.0 .. 1.0
            test_roundtrip( w1 );

            long double w2 = LDBL_MAX / rng(); // large values
            test_roundtrip( w2 );

            long double w3 = LDBL_MIN * rng(); // small values
            test_roundtrip( w3 );
        }

        test_roundtrip_bv<long double>();
    }

    // Selected additional values
    //
    test_roundtrip<double>(1.10393929655481808e+308);
    test_roundtrip<double>(-1.47902377240341038e+308);
    test_roundtrip<double>(-2.13177235460600904e+307);
    test_roundtrip<double>(8.60473951619578187e+307);
    test_roundtrip<double>(-2.97613696314797352e+306);

    test_roundtrip<float>(3.197633022e+38F);
    test_roundtrip<float>(2.73101834e+38F);
    test_roundtrip<float>(3.394053352e+38F);
    test_roundtrip<float>(5.549256619e+37F);
    test_roundtrip<float>(8.922125027e+34F);

#endif // Broken platforms

    return boost::report_errors();
}
