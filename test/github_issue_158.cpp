// Copyright 2024 Matt Borland
// Copyright 2024 Junekey Jeon
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/cppalliance/charconv/issues/158

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>

void test_values_with_negative_exp()
{
    char buffer[256];
    double d = 1e-15;
    auto res = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), d,
                         boost::charconv::chars_format::scientific, 50);
    *res.ptr = '\0';

    BOOST_TEST(res);
    BOOST_TEST_CSTR_EQ(buffer, "1.0000000000000000777053998766610792383071856011950e-15");

    res = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), d,
                         boost::charconv::chars_format::fixed, 50);
    *res.ptr = '\0';
    BOOST_TEST(res);
    BOOST_TEST_CSTR_EQ(buffer, "0.00000000000000100000000000000007770539987666107924");

    d = 1e-17;

    res = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), d,
                         boost::charconv::chars_format::fixed, 50);
    *res.ptr = '\0';
    BOOST_TEST(res);
    BOOST_TEST_CSTR_EQ(buffer, "1.00000000000000007154242405462192450852805618492325e-17");

    res = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), d,
                         boost::charconv::chars_format::fixed, 50);
    *res.ptr = '\0';
    BOOST_TEST(res);
    BOOST_TEST_CSTR_EQ(buffer, "0.00000000000000001000000000000000071542424054621925");
}

void test_values_with_positive_exp()
{
    char buffer[256];
    double d = 1e15;
    auto res = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), d,
                         boost::charconv::chars_format::scientific, 50);
    *res.ptr = '\0';

    BOOST_TEST(res);
    BOOST_TEST_CSTR_EQ(buffer, "1.00000000000000000000000000000000000000000000000000e+15");

    res = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), d,
                         boost::charconv::chars_format::fixed, 50);
    *res.ptr = '\0';
    BOOST_TEST(res);
    BOOST_TEST_CSTR_EQ(buffer, "1000000000000000.00000000000000000000000000000000000000000000000000");

    d = 1e17;

    res = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), d,
                         boost::charconv::chars_format::scientific, 50);
    *res.ptr = '\0';
    BOOST_TEST(res);
    BOOST_TEST_CSTR_EQ(buffer, "1.00000000000000000000000000000000000000000000000000e+17");

    res = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), d,
                         boost::charconv::chars_format::fixed, 50);
    *res.ptr = '\0';
    BOOST_TEST(res);
    BOOST_TEST_CSTR_EQ(buffer, "100000000000000000.00000000000000000000000000000000000000000000000000");
}

int main()
{
    //test_values_with_negative_exp();
    test_values_with_positive_exp();

    return boost::report_errors();
}
