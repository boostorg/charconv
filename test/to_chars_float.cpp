// Copyright 2018 Ulf Adams
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
#include <string>

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
void non_finite_values(boost::charconv::chars_format fmt = boost::charconv::chars_format::general, int precision = -1)
{
    char buffer1[256] {};
    T v1 = std::numeric_limits<T>::infinity();
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1), v1, fmt, precision);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "inf");

    char buffer2[256] {};
    T v2 = -std::numeric_limits<T>::infinity();
    auto r2 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer2), v2, fmt, precision);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer2, "-inf");

    char buffer3[256] {};
    T v3 = std::numeric_limits<T>::quiet_NaN();
    auto r3 = boost::charconv::to_chars(buffer3, buffer3 + sizeof(buffer3), v3, fmt, precision);
    BOOST_TEST_EQ(r3.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer3, "nan");

    char buffer4[256] {};
    T v4 = -std::numeric_limits<T>::quiet_NaN();
    auto r4 = boost::charconv::to_chars(buffer4, buffer4 + sizeof(buffer4), v4, fmt, precision);
    BOOST_TEST_EQ(r4.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer4, "-nan(ind)");

    char buffer5[256] {};
    T v5 = std::numeric_limits<T>::signaling_NaN();
    auto r5 = boost::charconv::to_chars(buffer5, buffer5 + sizeof(buffer5), v5, fmt, precision);
    BOOST_TEST_EQ(r5.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer5, "nan(snan)");

    char buffer6[256] {};
    T v6 = -std::numeric_limits<T>::signaling_NaN();
    auto r6 = boost::charconv::to_chars(buffer6, buffer6 + sizeof(buffer6), v6, fmt, precision);
    BOOST_TEST_EQ(r6.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer6, "-nan(snan)");
}

template <typename T>
void fixed_values()
{
    char buffer1[256] {};
    T v1 = 61851632;
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "61851632");
}

template <typename T>
void failing_ci_values()
{
    char buffer1[256] {};
    T v1 = -1.08260383390082946e+307;
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1), v1, boost::charconv::chars_format::hex);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "-1.ed5658af91a0fp+1019");

    char buffer2[256] {};
    T v2 = -9.52743282403084637e+306;
    auto r2 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer2), v2, boost::charconv::chars_format::hex);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer2, "-1.b22914956c56fp+1019");
}

template <typename T>
void spot_check(T v, const std::string& str, boost::charconv::chars_format fmt = boost::charconv::chars_format::general)
{
    char buffer[256] {};
    const auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), v, fmt);
    BOOST_TEST_EQ(r.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer, str.c_str());
}

int main()
{
    printf_divergence<double>();
    integer_general_format<double>();
    non_finite_values<double>();
    non_finite_values<double>(boost::charconv::chars_format::general, 2);
    non_finite_values<double>(boost::charconv::chars_format::scientific);
    non_finite_values<double>(boost::charconv::chars_format::scientific, 2);
    non_finite_values<double>(boost::charconv::chars_format::hex);
    non_finite_values<double>(boost::charconv::chars_format::hex, 2);

    fixed_values<float>();
    fixed_values<double>();

    failing_ci_values<double>();

    // Values from ryu tests
    spot_check(1.0, "1");
    spot_check(1.2, "1.2");
    spot_check(1.23, "1.23");
    spot_check(1.234, "1.234");
    spot_check(1.2345, "1.2345");
    spot_check(1.23456, "1.23456");
    spot_check(1.234567, "1.234567");
    spot_check(1.2345678, "1.2345678");
    spot_check(1.23456789, "1.23456789");
    spot_check(1.234567890, "1.23456789");
    spot_check(1.2345678901, "1.2345678901");
    spot_check(1.23456789012, "1.23456789012");
    spot_check(1.234567890123, "1.234567890123");
    spot_check(1.2345678901234, "1.2345678901234");
    spot_check(1.23456789012345, "1.23456789012345");
    spot_check(1.234567890123456, "1.234567890123456");

    spot_check(1.0, "1e+00", boost::charconv::chars_format::scientific);
    spot_check(1.2, "1.2e+00", boost::charconv::chars_format::scientific);
    spot_check(1.23, "1.23e+00", boost::charconv::chars_format::scientific);
    spot_check(1.234, "1.234e+00", boost::charconv::chars_format::scientific);
    spot_check(1.2345, "1.2345e+00", boost::charconv::chars_format::scientific);
    spot_check(1.23456, "1.23456e+00", boost::charconv::chars_format::scientific);
    spot_check(1.234567, "1.234567e+00", boost::charconv::chars_format::scientific);
    spot_check(1.2345678, "1.2345678e+00", boost::charconv::chars_format::scientific);
    spot_check(1.23456789, "1.23456789e+00", boost::charconv::chars_format::scientific);
    spot_check(1.234567890, "1.23456789e+00", boost::charconv::chars_format::scientific);
    spot_check(1.2345678901, "1.2345678901e+00", boost::charconv::chars_format::scientific);
    spot_check(1.23456789012, "1.23456789012e+00", boost::charconv::chars_format::scientific);
    spot_check(1.234567890123, "1.234567890123e+00", boost::charconv::chars_format::scientific);
    spot_check(1.2345678901234, "1.2345678901234e+00", boost::charconv::chars_format::scientific);
    spot_check(1.23456789012345, "1.23456789012345e+00", boost::charconv::chars_format::scientific);
    spot_check(1.234567890123456, "1.234567890123456e+00", boost::charconv::chars_format::scientific);

    spot_check(1.0, "1e+00", boost::charconv::chars_format::scientific);
    spot_check(12.0, "1.2e+01", boost::charconv::chars_format::scientific);
    spot_check(123.0, "1.23e+02", boost::charconv::chars_format::scientific);
    spot_check(1234.0, "1.234e+03", boost::charconv::chars_format::scientific);
    spot_check(12345.0, "1.2345e+04", boost::charconv::chars_format::scientific);
    spot_check(123456.0, "1.23456e+05", boost::charconv::chars_format::scientific);
    spot_check(1234567.0, "1.234567e+06", boost::charconv::chars_format::scientific);
    spot_check(12345678.0, "1.2345678e+07", boost::charconv::chars_format::scientific);
    spot_check(123456789.0, "1.23456789e+08", boost::charconv::chars_format::scientific);
    spot_check(1234567890.0, "1.23456789e+09", boost::charconv::chars_format::scientific);
    spot_check(12345678901.0, "1.2345678901e+10", boost::charconv::chars_format::scientific);
    spot_check(123456789012.0, "1.23456789012e+11", boost::charconv::chars_format::scientific);
    spot_check(1234567890123.0, "1.234567890123e+12", boost::charconv::chars_format::scientific);
    spot_check(12345678901234.0, "1.2345678901234e+13", boost::charconv::chars_format::scientific);
    spot_check(123456789012345.0, "1.23456789012345e+14", boost::charconv::chars_format::scientific);
    spot_check(1234567890123456.0, "1.234567890123456e+15", boost::charconv::chars_format::scientific);

    // Regressions or numbers that take >64 bits to represent correctly
    spot_check(9007199254740991.0, "9.007199254740991e+15", boost::charconv::chars_format::scientific);
    spot_check(9007199254740992.0, "9.007199254740992e+15", boost::charconv::chars_format::scientific);
    spot_check(123456789012345683968.0, "1.2345678901234568e+20", boost::charconv::chars_format::scientific);
    spot_check(1.9430376160308388E16, "1.9430376160308388e+16", boost::charconv::chars_format::scientific);
    spot_check(-6.9741824662760956E19, "-6.9741824662760956e+19", boost::charconv::chars_format::scientific);
    spot_check(4.3816050601147837E18, "4.3816050601147837e+18", boost::charconv::chars_format::scientific);
    spot_check(1.8531501765868567E21, "1.8531501765868567e+21", boost::charconv::chars_format::scientific);
    spot_check(-3.347727380279489E33, "-3.347727380279489e+33", boost::charconv::chars_format::scientific);
    spot_check(9.409340012568248E18, "9.409340012568248e+18", boost::charconv::chars_format::scientific);
    spot_check(4.708356024711512E18, "4.708356024711512e+18", boost::charconv::chars_format::scientific);
    spot_check(9.0608011534336E15, "9.0608011534336e+15", boost::charconv::chars_format::scientific);
    spot_check(2.989102097996E-312, "2.989102097996e-312", boost::charconv::chars_format::scientific);
    spot_check(1.18575755E-316, "1.18575755e-316", boost::charconv::chars_format::scientific);
    spot_check(4.940656E-318, "4.940656e-318", boost::charconv::chars_format::scientific);

    return boost::report_errors();
}
