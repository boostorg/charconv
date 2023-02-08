// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/limits.hpp>
#include <boost/charconv/detail/integer_search_trees.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>
#include <type_traits>

template <typename T>
void test()
{
    BOOST_TEST_GE(boost::charconv::limits<T>::max_chars10(), boost::charconv::detail::num_digits((std::numeric_limits<T>::max)()));
    BOOST_TEST_GE(static_cast<int>(sizeof(T) * CHAR_BIT), boost::charconv::limits<T>::max_chars());
}

int main(void)
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

    #ifdef BOOST_CHARCONV_HAS_INT128
    test<boost::charconv::int128_t>();
    test<boost::charconv::uint128_t>();
    #endif

    return boost::report_errors();
}
