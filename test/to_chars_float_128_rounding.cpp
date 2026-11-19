// Copyright 2026 mzfr
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstring>
#include <string>

template <typename T>
void test_format(T value, boost::charconv::chars_format fmt, int precision, const char* expected)
{
    char buffer[64];
    std::memset(buffer, '#', sizeof(buffer));
    const auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value, fmt, precision);
    BOOST_TEST(r);
    BOOST_TEST_EQ(std::string(buffer, r.ptr), expected);
}

template <typename T>
void test()
{
    test_format(static_cast<T>(9.94L), boost::charconv::chars_format::fixed, 1, "9.9");
    test_format(static_cast<T>(9.99L), boost::charconv::chars_format::fixed, 1, "10.0");
    test_format(static_cast<T>(-9.99L), boost::charconv::chars_format::fixed, 1, "-10.0");
    test_format(static_cast<T>(19.99L), boost::charconv::chars_format::fixed, 1, "20.0");
    test_format(static_cast<T>(99.99L), boost::charconv::chars_format::fixed, 1, "100.0");

    test_format(static_cast<T>(9.99L), boost::charconv::chars_format::general, 1, "1e+01");
    test_format(static_cast<T>(-9.99L), boost::charconv::chars_format::general, 1, "-1e+01");
    test_format(static_cast<T>(9.99L), boost::charconv::chars_format::scientific, 1, "1.0e+01");
    test_format(static_cast<T>(-9.99L), boost::charconv::chars_format::scientific, 1, "-1.0e+01");
}

int main()
{
    #if BOOST_CHARCONV_LDBL_BITS == 80
    test<long double>();
    #endif

    #ifdef BOOST_CHARCONV_HAS_QUADMATH
    test<__float128>();
    #endif

    #if defined(BOOST_CHARCONV_HAS_STDFLOAT128) && defined(BOOST_CHARCONV_HAS_QUADMATH)
    test<std::float128_t>();
    #endif

    return boost::report_errors();
}
