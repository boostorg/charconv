// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/config.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/detail/string_view.hpp>
#include <random>
#include <string>
#include <limits>

#if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
#  include <string_view>
#endif

static std::mt19937_64 rng(42);
constexpr std::size_t N = 1024;

template <typename T, typename StringViewType = boost::core::string_view>
void test_int()
{
    std::uniform_int_distribution<T> dist((std::numeric_limits<T>::min)(), (std::numeric_limits<T>::max)());

    for (std::size_t i = 0; i < N; ++i)
    {
        const auto value = dist(rng);
        std::string str_value = std::to_string(value);
        StringViewType sv = str_value;

        T v = 0;
        auto r = boost::charconv::from_chars(sv, v);
        BOOST_TEST(r);
        BOOST_TEST_EQ(v, value);
    }
}

int main()
{
    test_int<signed char>();
    test_int<unsigned char>();
    test_int<short>();
    test_int<unsigned short>();
    test_int<int>();
    test_int<unsigned>();
    test_int<long>();
    test_int<unsigned long>();
    test_int<long long>();
    test_int<unsigned long long>();

    #if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)

    test_int<signed char, std::string_view>();
    test_int<unsigned char, std::string_view>();
    test_int<short, std::string_view>();
    test_int<unsigned short, std::string_view>();
    test_int<int, std::string_view>();
    test_int<unsigned, std::string_view>();
    test_int<long, std::string_view>();
    test_int<unsigned long, std::string_view>();
    test_int<long long, std::string_view>();
    test_int<unsigned long long, std::string_view>();

    #endif

    return boost::report_errors();
}
