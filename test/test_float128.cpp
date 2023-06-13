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

            __float128 w1 = static_cast<__float128>( rng() * q ); // 0.0 .. 1.0
            test_roundtrip( w1 );

            __float128 w2 = FLT128_MAX / static_cast<__float128>( rng() ); // large values
            test_roundtrip( w2 );

            __float128 w3 = FLT128_MIN * static_cast<__float128>( rng() ); // small values
            test_roundtrip( w3 );
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
