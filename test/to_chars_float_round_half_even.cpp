// Copyright 2026 Pavel P
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>

static void test(const double value, const boost::charconv::chars_format fmt, const int precision, const char* expected)
{
    char buffer[128];
    const auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value, fmt, precision);
    BOOST_TEST(r);
    *r.ptr = '\0';

    BOOST_TEST_CSTR_EQ(buffer, expected);
}

static void test_examples()
{
    test(59547781175.354644775390625, boost::charconv::chars_format::fixed, 14, "59547781175.35464477539062");
    test(59547781175.354644775390625, boost::charconv::chars_format::scientific, 24, "5.954778117535464477539062e+10");
    test(-202364179515.495880126953125,boost::charconv::chars_format::fixed, 14, "-202364179515.49588012695312");
}

// m / 2^j has exactly j fractional digits, the last one 5, so dropping it is an exact tie
static void test_random_ties()
{
    std::mt19937_64 rng(42);
    for (int i {0}; i < 100000; ++i)
    {
        const std::uint64_t m {(rng() >> 11) | (UINT64_C(1) << 52) | 1};
        const int j {1 + static_cast<int>(rng() % 52)};
        const double value {std::ldexp(static_cast<double>(m), -j)};

        char exact[128];
        std::snprintf(exact, sizeof(exact), "%.*f", j, value);
        const int digits {static_cast<int>(std::strlen(exact)) - 1};

        char expected[128];
        std::snprintf(expected, sizeof(expected), "%.*f", j - 1, value);
        test(value, boost::charconv::chars_format::fixed, j - 1, expected);

        std::snprintf(expected, sizeof(expected), "%.*e", digits - 2, value);
        test(value, boost::charconv::chars_format::scientific, digits - 2, expected);
    }
}

int main()
{
    test_examples();
    test_random_ties();

    return boost::report_errors();
}
