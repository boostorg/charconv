// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/detail/parser.hpp>
#include <boost/charconv/chars_format.hpp>
#include <boost/core/lightweight_test.hpp>
#include <type_traits>
#include <cstdint>
#include <cstring>
#include <cerrno>

template <typename T>
void test_integer()
{
    std::uint64_t significand;
    std::int64_t  exponent;
    bool sign;

    const char* val1 = "12";
    auto r1 = boost::charconv::detail::parser<std::uint64_t, std::int64_t, T>(val1, val1 + std::strlen(val1), sign, significand, exponent);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(sign, false);
    BOOST_TEST_EQ(significand, 12);
    BOOST_TEST_EQ(exponent, 1);

    significand = 0;
    exponent = 0;
    sign = false;

    // Will exceed FLT_DIG (6 for IEEE float)
    const char* val2 = "123456789";
    auto r2 = boost::charconv::detail::parser<std::uint64_t, std::int64_t, T>(val2, val2 + std::strlen(val2), sign, significand, exponent);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_EQ(sign, false);
    BOOST_TEST_EQ(exponent, 8);
    BOOST_IF_CONSTEXPR(std::is_same<float, T>::value)
    {
        BOOST_TEST_EQ(significand, 123456);
    }
    else
    {
        BOOST_TEST_EQ(significand, 123456789);
    }

    auto r3 = boost::charconv::detail::parser<std::uint64_t, std::int64_t, T>(val2, val2 + std::strlen(val2), sign, significand, exponent, boost::charconv::chars_format::scientific);
    BOOST_TEST_EQ(r3.ec, EINVAL);
}

int main(void)
{
    test_integer<float>();
    test_integer<double>();
    test_integer<long double>();

    return boost::report_errors();
}
