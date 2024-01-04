// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#define BOOST_CHARCONV_STD_ERANGE
#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>

template <typename T>
void overflow_spot_value(const std::string& buffer, boost::charconv::chars_format fmt = boost::charconv::chars_format::general)
{
    auto v = static_cast<T>(42.L);
    auto r = boost::charconv::from_chars(buffer.c_str(), buffer.c_str() + std::strlen(buffer.c_str()), v, fmt);

    if (!(BOOST_TEST_EQ(v, static_cast<T>(42.L)) && BOOST_TEST(r.ec == std::errc::result_out_of_range)))
    {
        std::cerr << "Test failure for: " << buffer << " got: " << v << std::endl;
    }
}

template <typename T>
void test()
{
    const auto format_list = {boost::charconv::chars_format::fixed, boost::charconv::chars_format::general,
                              boost::charconv::chars_format::hex, boost::charconv::chars_format::scientific};

    for (const auto format : format_list)
    {
        overflow_spot_value<T>("1e99999", format);
        overflow_spot_value<T>("-1e99999", format);
        overflow_spot_value<T>("1e-99999", format);
        overflow_spot_value<T>("-1.0e-99999", format);
    }
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();

    #ifdef BOOST_CHARCONV_HAS_FLOAT128
    test<__float128>();
    #endif

    return boost::report_errors();
}
