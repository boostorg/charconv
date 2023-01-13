// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <charconv>
#include <type_traits>
#include <limits>
#include <cstring>
#include <cstdint>
#include <cerrno>
#include <utility>

template <typename T>
void test()
{
    // Base 2
    const char* base2 = "0101010";
    T base2_v_boost = 0;
    T base2_v_stl = 0;
    auto r1_boost = boost::charconv::from_chars(base2, base2 + std::strlen(base2), base2_v_boost, 2);
    auto r1_stl = std::from_chars(base2, base2 + std::strlen(base2), base2_v_stl, 2);
    BOOST_TEST_EQ(r1_boost.ptr, r1_stl.ptr);
    BOOST_TEST_EQ(base2_v_boost, base2_v_stl);

    // Hexadecimal
    const char* base16 = "0x2a";
    T base16_v_boost = 0;
    T base16_v_stl = 0;
    auto r2_boost = boost::charconv::from_chars(base16, base16 + std::strlen(base16), base16_v_boost, 16);
    auto r2_stl = std::from_chars(base16, base16 + std::strlen(base16), base16_v_stl, 16);
    BOOST_TEST_EQ(r2_boost.ptr, r2_stl.ptr);
    BOOST_TEST_EQ(base16_v_boost, base16_v_stl);
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
