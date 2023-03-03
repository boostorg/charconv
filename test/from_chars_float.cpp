// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <type_traits>
#include <limits>
#include <cstring>
#include <cstdint>
#include <cerrno>
#include <utility>

template <typename T>
void simple_integer_test()
{
    const char* buffer1 = "12";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, static_cast<T>(12));

    const char* buffer2 = "1200";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_EQ(v2, static_cast<T>(1200));
}

template <typename T>
void simple_hex_integer_test()
{
    const char* buffer1 = "-2a";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1, boost::charconv::chars_format::hex);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, static_cast<T>(-42));
}

template <typename T>
void simple_scientific_test()
{
    const char* buffer1 = "1e1";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, static_cast<T>(1e1L));

    const char* buffer2 = "123456789e10";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_EQ(v2, static_cast<T>(123456789e10L));

    const char* buffer3 = "1.23456789e+10";
    T v3 = 0;
    auto r3 = boost::charconv::from_chars(buffer3, buffer3 + std::strlen(buffer3), v3);
    BOOST_TEST_EQ(r3.ec, 0);
    BOOST_TEST_EQ(v3, static_cast<T>(1.23456789e+10L));

    const char* buffer4 = "1234.56789e+10";
    T v4 = 0;
    auto r4 = boost::charconv::from_chars(buffer4, buffer4 + std::strlen(buffer4), v4);
    BOOST_TEST_EQ(r4.ec, 0);
    BOOST_TEST_EQ(v4, static_cast<T>(1234.56789e+10L));
}

template <typename T>
void simple_hex_scientific_test()
{
    const char* buffer1 = "1.3a2bp-10";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1, boost::charconv::chars_format::hex);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, static_cast<T>(80427e-14L));

    const char* buffer2 = "1.234p-10";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2, boost::charconv::chars_format::hex);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_EQ(v2, static_cast<T>(4660e-13L));
}

template <typename T>
void dot_position_test()
{
    const char* buffer1 = "11.11111111";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, static_cast<T>(11.11111111L));

    const char* buffer2 = "1111.111111";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_EQ(v2, static_cast<T>(1111.111111L));

    const char* buffer3 = "111111.1111";
    T v3 = 0;
    auto r3 = boost::charconv::from_chars(buffer3, buffer3 + std::strlen(buffer3), v3);
    BOOST_TEST_EQ(r3.ec, 0);
    BOOST_TEST_EQ(v3, static_cast<T>(111111.1111L));

    const char* buffer4 = "1111111111.";
    T v4 = 0;
    auto r4 = boost::charconv::from_chars(buffer4, buffer4 + std::strlen(buffer4), v4);
    BOOST_TEST_EQ(r4.ec, 0);
    BOOST_TEST_EQ(v4, static_cast<T>(1111111111.L));
}

template <typename T>
void odd_strings_test()
{
    const char* buffer1 = "00000000000000000000000000000000000000000005";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, static_cast<T>(5));

    const char* buffer2 = "123456789123456789123456789";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_EQ(v2, static_cast<T>(1.23456789123456789123456789e26L));

    const char* buffer3 = "100000000000000000000000e5";
    T v3 = 0;
    auto r3 = boost::charconv::from_chars(buffer3, buffer3 + std::strlen(buffer3), v3);
    BOOST_TEST_EQ(r3.ec, 0);
    BOOST_TEST_EQ(v3, static_cast<T>(100000000000000000000000e5L));

    const char* buffer4 = "1.23456789123456789123456789123456789123456789e-5";
    T v4 = 0;
    auto r4 = boost::charconv::from_chars(buffer4, buffer4 + std::strlen(buffer4), v4);
    BOOST_TEST_EQ(r4.ec, 0);
    BOOST_TEST_EQ(v4, static_cast<T>(1.23456789123456789123456789123456789123456789e-5L));    
}

int main()
{
    simple_integer_test<float>();
    simple_integer_test<double>();
    
    simple_hex_integer_test<float>();
    simple_hex_integer_test<double>();

    simple_scientific_test<float>();
    simple_scientific_test<double>();

    simple_hex_scientific_test<float>();
    simple_hex_scientific_test<double>();

    dot_position_test<float>();
    dot_position_test<double>();

    odd_strings_test<float>();
    odd_strings_test<double>();

    #ifdef BOOST_CHARCONV_FULL_LONG_DOUBLE_IMPL
    simple_integer_test<long double>();
    simple_hex_integer_test<long double>();
    simple_scientific_test<long double>();
    simple_hex_scientific_test<long double>();
    #endif

    return boost::report_errors();
}
