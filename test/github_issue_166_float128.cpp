// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/boostorg/charconv/issues/166

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <string>

template <typename T>
void test()
{
    constexpr T value = 3746.348756384763L;
    constexpr int precision = 6;

    char buffer[1024];
    const auto result = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value, boost::charconv::chars_format::fixed, precision);
    *result.ptr = '\0';
    BOOST_TEST(result.ec == std::errc());
    BOOST_TEST_EQ(std::string{buffer}, std::to_string(3746.348756));
}

template <typename T>
void rounding()
{
    constexpr T value = 3746.348759784763L;
    constexpr int precision = 6;

    char buffer[1024];
    const auto result = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value, boost::charconv::chars_format::fixed, precision);
    *result.ptr = '\0';
    BOOST_TEST(result.ec == std::errc());
    BOOST_TEST_EQ(std::string{buffer}, std::to_string(3746.348760));
}

template <typename T>
void more_rounding()
{
    constexpr T value = 3746.89999999999999999L;
    constexpr int precision = 6;

    char buffer[1024];
    const auto result = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value, boost::charconv::chars_format::fixed, precision);
    *result.ptr = '\0';
    BOOST_TEST(result.ec == std::errc());
    BOOST_TEST_EQ(std::string{buffer}, std::to_string(3746.900000));
}

template <typename T>
void full_rounding_test()
{
    constexpr T value = 9999.999999999999999999L;
    constexpr int precision = 6;

    char buffer[1024];
    const auto result = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value, boost::charconv::chars_format::fixed, precision);
    *result.ptr = '\0';
    BOOST_TEST(result.ec == std::errc());
    BOOST_TEST_EQ(std::string{buffer}, std::to_string(10000.000000));
}

int main()
{
    #ifndef BOOST_CHARCONV_UNSUPPORTED_LONG_DOUBLE
    test<long double>();
    rounding<long double>();
    more_rounding<long double>();
    full_rounding_test<long double>();
    #endif

    #ifdef BOOST_CHARCONV_HAS_QUADMATH
    test<__float128>();
    rounding<__float128>();
    more_rounding<__float128>();
    full_rounding_test<__float128>();
    #endif

    #if defined(BOOST_CHARCONV_HAS_STDFLOAT128) && defined(BOOST_CHARCONV_HAS_QUADMATH)
    test<std::float128_t>();
    rounding<std::float128_t>();
    more_rounding<std::float128_t>();
    full_rounding_test<std::float128_t>();
    #endif

    return boost::report_errors();
}
