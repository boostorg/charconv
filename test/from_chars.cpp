// Copyright 2022 Peter Dimov
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
    static_assert(results.second.ec == 0, "No error");
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
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, 42);
}

template <typename T>
void base16_test()
{
    // In base 16 0x and 0X prefixes are ignored
    const char* buffer1 = "0x2a";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1, 16);
    BOOST_TEST_EQ(r1.ec, 0);
    BOOST_TEST_EQ(v1, 42);

    const char* buffer2 = "0";
    T v2 = 1;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2, 16);
    BOOST_TEST_EQ(r2.ec, 0);
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
        BOOST_TEST_EQ(r1.ec, ERANGE);
    }
    else
    {
        BOOST_TEST_EQ(r1.ec, 0) && BOOST_TEST_EQ(v1, 1234);
    }

    const char* buffer2 = "123456789123456789123456789";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2);
    // In the event of overflow v2 is to be returned unmodified
    BOOST_TEST_EQ(r2.ec, ERANGE) && BOOST_TEST_EQ(v2, 0);
}

template <typename T>
void invalid_argument_test()
{
    const char* buffer1 = "";
    T v1 = 0;
    auto r1 = boost::charconv::from_chars(buffer1, buffer1 + std::strlen(buffer1), v1);
    BOOST_TEST_EQ(r1.ec, EINVAL);

    const char* buffer2 = "-";
    T v2 = 0;
    auto r2 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer2), v2);
    BOOST_TEST_EQ(r2.ec, EINVAL);

    const char* buffer3 = "+";
    T v3 = 0;
    auto r3 = boost::charconv::from_chars(buffer3, buffer3 + std::strlen(buffer3), v3);
    BOOST_TEST_EQ(r3.ec, EINVAL);

    BOOST_IF_CONSTEXPR(std::is_unsigned<T>::value)
    {
        const char* buffer4 = "-123";
        T v4 = 0;
        auto r4 = boost::charconv::from_chars(buffer4, buffer4 + std::strlen(buffer4), v4);
        BOOST_TEST_EQ(r4.ec, EINVAL);
    }
}

// No overflows, negative numbers, locales, etc.
template <typename T>
void simple_test()
{
    const char* buffer = "34";

    T v = 0;
    auto r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), v);

    BOOST_TEST_EQ( r.ec, 0 ) && BOOST_TEST_EQ(v, 34);
    BOOST_TEST(r == r);

    boost::charconv::from_chars_result r2 {r.ptr, 0};
    BOOST_TEST(r == r2);

    const char* buffer2 = "12";
    T v2 = 0;
    auto r3 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer), v2);
    BOOST_TEST(r != r3);
    BOOST_TEST_EQ(r3.ec, 0) && BOOST_TEST_EQ(v2, 12);
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

    return boost::report_errors();
}
