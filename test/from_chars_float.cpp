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
}

template <typename T>
void simple_hex_scientifc_test()
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

int main()
{
    simple_integer_test<float>();
    simple_integer_test<double>();
    
    simple_hex_integer_test<float>();
    simple_hex_integer_test<double>();

    simple_scientific_test<float>();
    simple_scientific_test<double>();

    simple_hex_scientifc_test<float>();
    simple_hex_scientifc_test<double>();

    #ifdef BOOST_CHARCONV_FULL_LONG_DOUBLE_IMPL
    simple_integer_test<long double>();
    simple_hex_integer_test<long double>();
    simple_scientific_test<long double>();
    simple_hex_scientifc_test<long double>();
    #endif

    return boost::report_errors();
}
