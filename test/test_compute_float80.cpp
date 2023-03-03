// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/detail/compute_float80.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <limits>
#include <cstdint>
#include <cmath>

// MSVC uses long double = double
// Darwin sometimes uses double double instead of long double
#if BOOST_CHARCONV_LDBL_BITS == 80 && !defined(__APPLE__) && !defined(_WIN32) && !defined(_WIN64)

using boost::charconv::detail::compute_float80;

inline void simple_test()
{
    bool success;

    // Trivial verifcation
    BOOST_TEST_EQ(compute_float80(1, 1, false, success), 1e1L);
    BOOST_TEST_EQ(compute_float80(0, 1, true, success), -1e0L);
    BOOST_TEST_EQ(compute_float80(308, 1, false, success), 1e308L);

    // out of range
    BOOST_TEST_EQ(compute_float80(10000, 5, false, success), 0L); 
    BOOST_TEST_EQ(compute_float80(-10000, 5, false, success), 0L);

    // Composite
    BOOST_TEST_EQ(compute_float80(10, 123456789, false, success), 123456789e10L);
    BOOST_TEST_EQ(compute_float80(10, 123456789, true, success), -123456789e10L);
    BOOST_TEST_EQ(compute_float80(100, UINT64_C(4444444444444444444), false, success), 4444444444444444444e100L);
}

int main(void)
{
    simple_test();

    return boost::report_errors();
}

#else
int main(void)
{
    return 0;
}
#endif
