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
    char buffer1[64] {};
    T v1 = static_cast<T>(-0);
    auto r1 = std::to_chars(buffer1, buffer1 + sizeof(buffer1) - 1, v1);
    (void)r1;
    BOOST_TEST_CSTR_EQ(buffer1, "0");
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
