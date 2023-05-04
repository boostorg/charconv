// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cmath>

template <typename T>
void spot_value(const std::string& buffer, T expected_value)
{
    T v = 0;
    auto r = boost::charconv::from_chars(buffer.c_str(), buffer.c_str() + std::strlen(buffer.c_str()), v);
    BOOST_TEST_EQ(r.ec, 0);
    if (!BOOST_TEST_EQ(v, expected_value))
    {
        std::cerr << "Test failure for: " << buffer << " got: " << v << std::endl;
    }
}

void fc (const std::string& s)
{
    char* str_end;
    const double expected_value = std::strtod(s.c_str(), &str_end);
    spot_value(s, expected_value);
}

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

    const char* buffer5 = "1.23456789123456789123456789123456789123456789e-00000000000000000005";
    T v5 = 0;
    auto r5 = boost::charconv::from_chars(buffer5, buffer5 + std::strlen(buffer5), v5);
    BOOST_TEST_EQ(r5.ec, 0);
    BOOST_TEST_EQ(v5, static_cast<T>(1.23456789123456789123456789123456789123456789e-5L));

}

template <typename T>
void zero_test()
{
    const char* buffer1 = "0e0";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, static_cast<T>(0));
    BOOST_TEST(!std::signbit(v1));

    const char* buffer2 = "-0e0";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_EQ(v2, static_cast<T>(-0));
    BOOST_TEST(std::signbit(v2));

    const char* buffer3 = "0.0";
    T v3 = 0;
    auto r3 = boost::charconv::from_chars(buffer3, buffer3 + std::strlen(buffer3), v3);
    BOOST_TEST_EQ(r3.ec, 0);
    BOOST_TEST_EQ(v3, static_cast<T>(0.0));
    BOOST_TEST(!std::signbit(v3));

    const char* buffer4 = "-0.0";
    T v4 = 0;
    auto r4 = boost::charconv::from_chars(buffer4, buffer4 + std::strlen(buffer4), v4);
    BOOST_TEST_EQ(r4.ec, 0);
    BOOST_TEST_EQ(v4, static_cast<T>(-0));
    BOOST_TEST(std::signbit(v4));

    const char* buffer5 = "0";
    T v5 = 0;
    auto r5 = boost::charconv::from_chars(buffer5, buffer5 + std::strlen(buffer5), v5);
    BOOST_TEST_EQ(r5.ec, 0);
    BOOST_TEST_EQ(v5, static_cast<T>(0));
    BOOST_TEST(!std::signbit(v5));

    const char* buffer6 = "-0";
    T v6 = 0;
    auto r6 = boost::charconv::from_chars(buffer6, buffer6 + std::strlen(buffer6), v6);
    BOOST_TEST_EQ(r6.ec, 0);
    BOOST_TEST_EQ(v6, static_cast<T>(-0));
    BOOST_TEST(std::signbit(v6));
}

template <typename T>
void boost_json_test()
{
    const char* buffer1 = "-0.010";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, static_cast<T>(-0.01L));

    fc("-0.9999999999999999999999");
    fc("-0.9999999999999999");
    fc("-0.9007199254740991");
    fc("-0.999999999999999");
    fc("-0.99999999999999");
    fc("-0.9999999999999");
    fc("-0.999999999999");
    fc("-0.99999999999");
    fc("-0.9999999999");
    fc("-0.999999999");
    fc("-0.99999999");
    fc("-0.9999999");
    fc("-0.999999");
    fc("-0.99999");
    fc("-0.9999");
    fc("-0.8125");
    fc("-0.999");
    fc("-0.99");
    fc("-1.0");
    fc("-0.9");
    fc("-0.0");
    fc("0.0");
    fc("0.9");
    fc("0.99");
    fc("0.999");
    fc("0.8125");
    fc("0.9999");
    fc("0.99999");
    fc("0.999999");
    fc("0.9999999");
    fc("0.99999999");
    fc("0.999999999");
    fc("0.9999999999");
    fc("0.99999999999");
    fc("0.999999999999");
    fc("0.9999999999999");
    fc("0.99999999999999");
    fc("0.999999999999999");
    fc("0.9007199254740991");
    fc("0.9999999999999999");
    fc("0.9999999999999999999999");
    fc("0.999999999999999999999999999");

    fc("-1e308");
    fc("-1e-308");
    fc("-9999e300");
    fc("-999e100");
    fc("-99e10");
    fc("-9e1");
    fc("9e1");
    fc("99e10");
    fc("999e100");
    fc("9999e300");
    fc("999999999999999999.0");
    fc("999999999999999999999.0");
    fc("999999999999999999999e5");
    fc("999999999999999999999.0e5");

    fc("0.00000000000000001");

    fc("-1e-1");
    fc("-1e0");
    fc("-1e1");
    fc("0e0");
    fc("1e0");
    fc("1e10");

    fc("0."
       "00000000000000000000000000000000000000000000000000" // 50 zeroes
       "1e50");
    fc("-0."
       "00000000000000000000000000000000000000000000000000" // 50 zeroes
       "1e50");

    fc("0."
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000" // 100 zeroes
       "1e100");
    fc("-0."
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000" // 100 zeroes
       "1e100");

    fc("0."
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000" // 500 zeroes
       "1e600");
    fc("-0."
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000"
       "00000000000000000000000000000000000000000000000000" // 500 zeroes
       "1e600");
/*
    spot_value("-1.010", -1.01);
    spot_value("-0.010", -0.01);
    spot_value("-0.0", -0.0);
    spot_value("-0e0", -0.0);
    spot_value("18.4",  18.4);
    spot_value("-18.4", -18.4);
    spot_value("18446744073709551616",  1.8446744073709552e+19);
    spot_value("-18446744073709551616", -1.8446744073709552e+19);
    spot_value("18446744073709551616.0",  1.8446744073709552e+19);
    spot_value("18446744073709551616.00009",  1.8446744073709552e+19);
    spot_value("1844674407370955161600000",  1.8446744073709552e+24);
    spot_value("-1844674407370955161600000", -1.8446744073709552e+24);
    spot_value("1844674407370955161600000.0",  1.8446744073709552e+24);
    spot_value("1844674407370955161600000.00009",  1.8446744073709552e+24);
    spot_value("19700720435664.186294290058937593e13",  1.9700720435664185e+26);

    spot_value("1.0", 1.0);
    spot_value("1.1", 1.1);
    spot_value("1.11", 1.11);
    spot_value("1.11111", 1.11111);
    spot_value("11.1111", 11.1111);
    spot_value("111.111", 111.111);
*/
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

    zero_test<float>();
    zero_test<double>();

    boost_json_test<double>();

    return boost::report_errors();
}
