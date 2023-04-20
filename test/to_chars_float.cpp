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

// These numbers diverge from what the formatting is using printf
// See: https://godbolt.org/z/zd34KcWMW
template <typename T>
void printf_divergence()
{
    char buffer1[256] {};
    T v1 = 3.4;
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "3.4");

    char buffer2[256] {};
    T v2 = 3000.40;
    auto r2 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer2), v2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer2, "3000.4");

    char buffer3[256] {};
    T v3 = -3000000300000000.5;
    auto r3 = boost::charconv::to_chars(buffer3, buffer3 + sizeof(buffer3), v3);
    BOOST_TEST_EQ(r3.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer3, "-3000000300000000.5");
}

template <typename T>
void integer_general_format()
{
    char buffer1[256] {};
    T v1 = 1217.2772861138403;
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "1217.2772861138403");
    T return_v1;
    auto r1_return = boost::charconv::from_chars(buffer1, buffer1 + strlen(buffer1), return_v1);
    BOOST_TEST_EQ(r1_return.ec, 0);
    BOOST_TEST_EQ(return_v1, v1);
}

template <typename T>
void non_finite_values()
{
    char buffer1[256] {};
    T v1 = std::numeric_limits<T>::infinity();
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "inf");

    char buffer2[256] {};
    T v2 = -std::numeric_limits<T>::infinity();
    auto r2 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer2), v2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer2, "-inf");

    char buffer3[256] {};
    T v3 = std::numeric_limits<T>::quiet_NaN();
    auto r3 = boost::charconv::to_chars(buffer3, buffer3 + sizeof(buffer3), v3);
    BOOST_TEST_EQ(r3.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer3, "nan");
}

int main()
{
    printf_divergence<double>();
    integer_general_format<double>();
    non_finite_values<double>();

    return boost::report_errors();
}
