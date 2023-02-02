// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <type_traits>
#include <limits>
#include <string>
#include <cstring>
#include <cerrno>

void off_by_one_tests(int value)
{
    char buffer[64] {};
    auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer) - 1, value);
    BOOST_TEST_EQ(r.ec, 0);
    std::string value_string = std::to_string(value);
    BOOST_TEST_CSTR_EQ(buffer, value_string.c_str());
}

template <typename T>
void base_thirtytwo_tests()
{
    char buffer1[64] {};
    T v1 = static_cast<T>(42);
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1, 32);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "1a");
}

template <typename T>
void base_sixteen_tests()
{
    char buffer1[64] {};
    T v1 = static_cast<T>(42);
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1, 16);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "2a");
}

template <typename T>
void base_eight_tests()
{
    char buffer1[64] {};
    T v1 = static_cast<T>(42);
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1, 8);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "52");
}

template <typename T>
void base_four_tests()
{
    char buffer1[64] {};
    T v1 = static_cast<T>(42);
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1, 4);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "222");
}

// Tests the generic implementation
template <typename T>
void base_30_tests()
{
    char buffer1[64] {};
    T v1 = static_cast<T>(1234);
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1, 30);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "1b4");

    char buffer2[64] {};
    T v2 = static_cast<T>(-4321);
    auto r2 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer2) - 1, v2, 30);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer2, "-4o1");
}

template <typename T>
void overflow_tests()
{
    char buffer1[2] {};
    T v1 = static_cast<T>(250);
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1);
    BOOST_TEST_EQ(r1.ec, EOVERFLOW);

    char buffer2[3] {};
    T v2 = static_cast<T>(12341234);
    auto r2 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer2) - 1, v2);
    BOOST_TEST_EQ(r2.ec, EOVERFLOW);
}

template <typename T>
void base_two_tests()
{
    char buffer1[64] {};
    T v1 = static_cast<T>(42);
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1, 2);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "101010");
}

template <typename T>
void sixty_four_bit_tests()
{
    char buffer1[64] {};
    T v1 = static_cast<T>(-1234);
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "-1234");

    char buffer2[64] {};
    T v2 = static_cast<T>(1234123412341234LL);
    auto r2 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer2) - 1, v2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer2, "1234123412341234");
}

template <>
void sixty_four_bit_tests<std::uint64_t>()
{
    char buffer1[64] {};
    std::uint64_t v1 = (std::numeric_limits<std::uint64_t>::max)();
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "18446744073709551615");

    // Cutting this value in half would overflow a 32 bit unsigned for the back 10 digits
    char buffer2[64] {};
    std::uint64_t v2 = UINT64_C(9999999999999999999);
    auto r2 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer2) - 1, v2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer2, "9999999999999999999");

    // Account for zeros in the back half of the split
    char buffer3[64] {};
    std::uint64_t v3 = UINT64_C(10000000000000000000);
    auto r3 = boost::charconv::to_chars(buffer3, buffer3 + sizeof(buffer3) - 1, v3);
    BOOST_TEST_EQ(r3.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer3, "10000000000000000000");
}

template <typename T>
void negative_vals_test()
{
    char buffer1[10] {};
    T v = -4321;
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "-4321");
}

template <typename T>
void simple_test()
{
    char buffer1[64] {};
    T v = 34;
    auto r1 = boost::charconv::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer1, "34");

    boost::charconv::to_chars_result r {r1.ptr, r1.ec};
    BOOST_TEST(r1 == r);

    char buffer2[64] {};
    T v2 = 12;
    auto r2 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer2) - 1, v2);
    BOOST_TEST(r1 != r2);
    BOOST_TEST_EQ(r2.ec, 0);
    BOOST_TEST_CSTR_EQ(buffer2, "12");
}

int main()
{
    simple_test<char>();
    simple_test<signed char>();
    simple_test<unsigned char>();
    simple_test<short>();
    simple_test<unsigned short>();
    simple_test<int>();
    simple_test<unsigned>();
    simple_test<long>();
    simple_test<unsigned long>();
    simple_test<long long>();
    simple_test<unsigned long long>();
    simple_test<std::int32_t>();
    simple_test<std::uint64_t>();

    negative_vals_test<int>();
    negative_vals_test<long>();

    sixty_four_bit_tests<long long>();
    sixty_four_bit_tests<std::uint64_t>();

    // Tests for every specialized base
    base_two_tests<int>();
    base_two_tests<unsigned>();

    base_four_tests<int>();
    base_four_tests<unsigned>();

    base_eight_tests<int>();
    base_eight_tests<unsigned>();

    base_sixteen_tests<int>();
    base_sixteen_tests<unsigned>();

    base_thirtytwo_tests<int>();
    base_thirtytwo_tests<unsigned>();

    // The generic impl
    base_30_tests<int>();
    base_30_tests<long>();

    overflow_tests<int>();

    // Resulted in off by one errors from random number generation
    // Consistently one larger with 10 digit numbers
    off_by_one_tests(1159137169);
    off_by_one_tests(-1321793318);
    off_by_one_tests(2140634902);
    
    // If we compensate by one these fail
    off_by_one_tests(1038882992);
    off_by_one_tests(-1065658613);
    off_by_one_tests(-1027205339);

    return boost::report_errors();
}
