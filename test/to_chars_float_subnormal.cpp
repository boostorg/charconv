// Copyright 2026 Pavel P
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <random>

static void test(const double value, const boost::charconv::chars_format fmt, const int precision, const char* expected)
{
    char buffer[64];
    const auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value, fmt, precision);
    BOOST_TEST(r);
    *r.ptr = '\0';

    BOOST_TEST_CSTR_EQ(buffer, expected);
}

// Subnormals whose first digit segment has an odd number of digits
static void test_examples()
{
    using boost::charconv::chars_format;

    test(6.654853158546871e-310, chars_format::scientific, 15, "6.654853158546871e-310");
    test(1.5e-312, chars_format::scientific, 15, "1.500000000000168e-312");
    test(4.9406564584124654e-324, chars_format::scientific, 15, "4.940656458412465e-324");
    test(1.5e-311, chars_format::scientific, 15, "1.500000000000168e-311");

    test(6.654853158546871e-310, chars_format::scientific, 0, "7e-310");
    test(6.654853158546871e-310, chars_format::scientific, 1, "6.7e-310");
    test(6.654853158546871e-310, chars_format::general, 15, "6.65485315854687e-310");
}

// Every subnormal must print with the digits and exponent printf gives
static void test_random_subnormals()
{
    std::mt19937_64 rng(42);
    for (int i {}; i < 100000; ++i)
    {
        const std::uint64_t bits = (rng() & ((UINT64_C(1) << 52U) - 1U)) >> (rng() % 52U);
        if (bits == 0U)
        {
            continue;
        }

        double value {};
        std::memcpy(&value, &bits, sizeof(value));

        for (const int precision : {0, 1, 6})
        {
            char expected[64];
            std::snprintf(expected, sizeof(expected), "%.*e", precision, value);
            test(value, boost::charconv::chars_format::scientific, precision, expected);
        }
    }
}

int main()
{
    test_examples();
    test_random_subnormals();

    return boost::report_errors();
}
