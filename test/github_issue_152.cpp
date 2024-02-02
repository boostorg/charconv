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
