// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/config.hpp>

#if !defined(BOOST_NO_CXX17_HDR_CHARCONV) && (!defined(__clang_major__) || (defined(__clang_major__) && __clang_major__ > 7))

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <charconv>
#include <type_traits>
#include <limits>
#include <cstring>
#include <cstdint>
#include <cerrno>
#include <utility>
#include <system_error>

template <typename T>
void test()
{
    // Signed 0
    char buffer1_stl[64] {};
    char buffer1_boost[64] {};
    T v1 = static_cast<T>(-0);
    auto r1_stl = std::to_chars(buffer1_stl, buffer1_stl + sizeof(buffer1_stl) - 1, v1);
    auto r1_boost = boost::charconv::to_chars(buffer1_boost, buffer1_boost + sizeof(buffer1_boost) - 1, v1);
    BOOST_TEST_EQ(r1_stl.ptr, buffer1_stl + 1);
    BOOST_TEST_EQ(r1_boost.ptr, buffer1_boost + 1);
    BOOST_TEST_CSTR_EQ(buffer1_stl, "0");
    BOOST_TEST_CSTR_EQ(buffer1_boost, "0");

    // Binary
    char buffer2_stl[64] {};
    char buffer2_boost[64] {};
    T v2 = static_cast<T>(42);
    auto r2_stl = std::to_chars(buffer2_stl, buffer2_stl + sizeof(buffer2_stl) - 1, v2, 2);
    auto r2_boost = boost::charconv::to_chars(buffer2_boost, buffer2_boost + sizeof(buffer2_boost) - 1, v2, 2);
    BOOST_TEST_EQ(r2_stl.ptr, buffer2_stl + 6);
    BOOST_TEST_EQ(r2_boost.ptr, buffer2_boost + 6);
    BOOST_TEST_CSTR_EQ(buffer2_stl, "101010");
    BOOST_TEST_CSTR_EQ(buffer2_boost, "101010");

    // Base 10
    char buffer3_stl[64] {};
    char buffer3_boost[64] {};
    T v3 = static_cast<T>(120);
    auto r3_stl = std::to_chars(buffer3_stl, buffer3_stl + sizeof(buffer3_stl) - 1, v3);
    auto r3_boost = boost::charconv::to_chars(buffer3_boost, buffer3_boost + sizeof(buffer3_boost) - 1, v3);
    BOOST_TEST_EQ(r3_stl.ptr, buffer3_stl + 3);
    BOOST_TEST_EQ(r3_boost.ptr, buffer3_boost + 3);
    BOOST_TEST_CSTR_EQ(buffer3_stl, "120");
    BOOST_TEST_CSTR_EQ(buffer3_boost, "120");

    // Hexadecimal
    char buffer4_stl[64] {};
    char buffer4_boost[64] {};
    T v4 = static_cast<T>(44);
    auto r4_stl = std::to_chars(buffer4_stl, buffer4_stl + sizeof(buffer4_stl) - 1, v4, 16);
    auto r4_boost = boost::charconv::to_chars(buffer4_boost, buffer4_boost + sizeof(buffer4_boost) - 1, v4, 16);
    BOOST_TEST_EQ(r4_stl.ptr, buffer4_stl + 2);
    BOOST_TEST_EQ(r4_boost.ptr, buffer4_boost + 2);
    BOOST_TEST_CSTR_EQ(buffer4_stl, "2c");
    BOOST_TEST_CSTR_EQ(buffer4_boost, "2c");

}

int main()
{
    test<char>();
    test<signed char>();
    test<unsigned char>();
    test<short>();
    test<unsigned short>();
    test<int>();
    test<unsigned>();
    test<long>();
    test<unsigned long>();
    test<long long>();
    test<unsigned long long>();

    return boost::report_errors();
}

#else

int main()
{ 
    return 0;
}

#endif
