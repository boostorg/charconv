// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <system_error>
#include <limits>

template <typename T>
void test_non_finite()
{
    auto value = {std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity(),
                  std::numeric_limits<T>::quiet_NaN(), -std::numeric_limits<T>::quiet_NaN(),
                  std::numeric_limits<T>::signaling_NaN(), -std::numeric_limits<T>::signaling_NaN()};

    for (const auto val : value)
    {
        char buffer[2];
        auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), val);
        BOOST_TEST(r.ec == std::errc::result_out_of_range);
    }

    char inf_buffer[3];
    auto r_inf = boost::charconv::to_chars(inf_buffer, inf_buffer + 3, std::numeric_limits<T>::infinity());
    BOOST_TEST(r_inf);
    BOOST_TEST(!std::memcmp(inf_buffer, "inf", 3));

    char nan_buffer[3];
    auto r_nan = boost::charconv::to_chars(nan_buffer, nan_buffer + 3, std::numeric_limits<T>::quiet_NaN());
    BOOST_TEST(r_nan);
    BOOST_TEST(!std::memcmp(nan_buffer, "nan", 3));

    char neg_nan_buffer[9];
    auto r_neg_nan = boost::charconv::to_chars(neg_nan_buffer, neg_nan_buffer + 9, -std::numeric_limits<T>::quiet_NaN());
    BOOST_TEST(r_neg_nan);
    BOOST_TEST(!std::memcmp(neg_nan_buffer, "-nan(ind)", 9));

    char snan_buffer[9];
    auto r_snan = boost::charconv::to_chars(snan_buffer, snan_buffer + 9, std::numeric_limits<T>::signaling_NaN());
    BOOST_TEST(r_snan);
    BOOST_TEST(!std::memcmp(snan_buffer, "nan(snan)", 9));
};

int main()
{
    test_non_finite<float>();
    test_non_finite<double>();
    test_non_finite<long double>();

    #ifdef BOOST_CHARCONV_HAS_FLOAT128
    test_non_finite<__float128>();
    #endif

    return boost::report_errors();
}