// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <type_traits>
#include <limits>
#include <cstring>
#include <cerrno>

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

    base_two_tests<int>();
    base_two_tests<unsigned>();

    return boost::report_errors();
}
