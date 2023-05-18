// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <system_error>
#include <type_traits>
#include <limits>
#include <cstring>
#include <cstdint>
#include <cerrno>
#include <utility>

#ifdef __GLIBCXX_TYPE_INT_N_0
template <typename T>
void test_128bit_int()
{
    const char* buffer1 = "85070591730234615865843651857942052864"; // 2^126
    T test_value = 1;
    test_value = test_value << 126;
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);
    BOOST_TEST(r1.ec == std::errc());
    BOOST_TEST(v1 == test_value);
    BOOST_TEST(std::numeric_limits<T>::max() > static_cast<T>(std::numeric_limits<unsigned long long>::max()));
}
#endif // 128-bit testing

#ifndef BOOST_NO_CXX14_CONSTEXPR
template <typename T>
constexpr std::pair<T, boost::charconv::from_chars_result> constexpr_test_helper()
{
    const char* buffer1 = "42";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + 2, v1);

    return std::make_pair(v1, r1);
}

template <typename T>
constexpr void constexpr_test()
{
    constexpr auto results = constexpr_test_helper<T>();
    static_assert(results.second.ec == std::errc(), "No error");
    static_assert(results.first == 42, "Value is 42");
}

#endif

template <typename T>
void base2_test()
{
    // Includes leading 0 which should be ignored
    const char* buffer1 = "0101010";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1, 2);
    BOOST_TEST(r1.ec == std::errc());
    BOOST_TEST_EQ(v1, 42);
}

template <typename T>
void base16_test()
{
    // In base 16 0x and 0X prefixes are not allowed
    const char* buffer1 = "2a";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1, 16);
    BOOST_TEST(r1.ec == std::errc());
    BOOST_TEST_EQ(v1, 42);

    const char* buffer2 = "0";
    T v2 = 1;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2, 16);
    BOOST_TEST(r2.ec == std::errc());
    BOOST_TEST_EQ(v2, 0);
}

template <typename T>
void overflow_test()
{
    const char* buffer1 = "1234";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);

    BOOST_IF_CONSTEXPR((std::numeric_limits<T>::max)() < 1234)
    {
        BOOST_TEST(r1.ec == std::errc::result_out_of_range);
    }
    else
    {
        BOOST_TEST(r1.ec == std::errc()) && BOOST_TEST_EQ(v1, 1234);
    }

    const char* buffer2 = "123456789123456789123456789";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2);
    // In the event of overflow v2 is to be returned unmodified
    BOOST_TEST(r2.ec == std::errc::result_out_of_range) && BOOST_TEST_EQ(v2, 0);
}

template <typename T>
void invalid_argument_test()
{
    const char* buffer1 = "";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);
    BOOST_TEST(r1.ec == std::errc::invalid_argument);

    const char* buffer2 = "-";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2);
    BOOST_TEST(r2.ec == std::errc::invalid_argument);

    const char* buffer3 = "+";
    T v3 = 0;
    auto r3 = boost::charconv::from_chars(buffer3, buffer3 + std::strlen(buffer3), v3);
    BOOST_TEST(r3.ec == std::errc::invalid_argument);

    BOOST_IF_CONSTEXPR(std::is_unsigned<T>::value)
    {
        const char* buffer4 = "-123";
        T v4 = 0;
        auto r4 = boost::charconv::from_chars(buffer4, buffer4 + std::strlen(buffer4), v4);
        BOOST_TEST(r4.ec == std::errc::invalid_argument);
    }

    // Bases outside 2-36 inclusive return std::errc::invalid_argument
    const char* buffer5 = "23";
    T v5 = 0;
    auto r5 = boost::charconv::from_chars(buffer5, buffer5 + std::strlen(buffer5), v5, 1);
    BOOST_TEST(r5.ec == std::errc::invalid_argument);
    auto r6 = boost::charconv::from_chars(buffer5, buffer5 + std::strlen(buffer5), v5, 50);
    BOOST_TEST(r6.ec == std::errc::invalid_argument);

    const char* buffer7 = "+12345";
    T v7 = 3;
    auto r7 = boost::charconv::from_chars(buffer7, buffer7 + std::strlen(buffer7), v7);
    BOOST_TEST(r7.ec == std::errc::invalid_argument);
    BOOST_TEST_EQ(v7, 3);
}

// No overflows, negative numbers, locales, etc.
template <typename T>
void simple_test()
{
    const char* buffer = "34";

    T v = 0;
    auto r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), v);

    BOOST_TEST( r.ec == std::errc() ) && BOOST_TEST_EQ(v, 34);
    BOOST_TEST(r == r);

    boost::charconv::from_chars_result r2 {r.ptr, std::errc()};
    BOOST_TEST(r == r2);

    const char* buffer2 = "12";
    T v2 = 0;
    auto r3 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer), v2);
    BOOST_TEST(r != r3);
    BOOST_TEST(r3.ec == std::errc()) && BOOST_TEST_EQ(v2, 12);
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
    
    invalid_argument_test<int>();
    invalid_argument_test<unsigned>();

    overflow_test<char>();
    overflow_test<int>();

    base16_test<int>();
    base16_test<unsigned>();

    base2_test<unsigned char>();
    base2_test<long>();

    #if !(defined(__GNUC__) && __GNUC__ == 5)
    #   ifndef BOOST_NO_CXX14_CONSTEXPR
            constexpr_test<int>();
    #   endif
    #endif

    // Only compiles using cxxstd-dialect=gnu or equivalent
    #ifdef __GLIBCXX_TYPE_INT_N_0
    test_128bit_int<__int128>();
    test_128bit_int<unsigned __int128>();
    #endif

    return boost::report_errors();
}
