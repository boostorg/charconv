// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/detail/leading_zeros.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>
#include <cstdint>

template <typename T>
void test()
{
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(1), 63);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(2), 62);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(3), 62);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(4), 61);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(8), 60);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(16), 59);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(32), 58);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(64), 57);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(128), 56);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(200), 56);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(256), 55);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(500), 55);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(512), 54);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(1000), 54);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(1024), 53);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max() - 1), 0);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max()), 0);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max() / 2 + 1), 0);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max() / 2), 1);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max() / 4 + 1), 1);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max() / 4), 2);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max() / 8), 3);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max() / 16), 4);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max() / 32), 5);
    BOOST_TEST_EQ(boost::charconv::detail::leading_zeros(std::numeric_limits<T>::max() / 64), 6);
}

int main(void)
{
    test<std::uint64_t>();

    return boost::report_errors();
}
