// Copyright 2026 Pavel P
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// Scientific output with a precision past the first 17 or so digits, compared with printf

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>

static double from_bits(const std::uint64_t bits)
{
    double value {};
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

static void test(const double value, const int precision, const char* expected)
{
    char buffer[128];
    const auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value,
                                             boost::charconv::chars_format::scientific, precision);
    BOOST_TEST(r);
    *r.ptr = '\0';

    BOOST_TEST_CSTR_EQ(buffer, expected);
}

// Whether the digits past the requested ones are exactly 5000..., where printf rounds to even
static bool is_tie(const double value, const int precision)
{
    char longer[160];
    std::snprintf(longer, sizeof(longer), "%.*e", precision + 60, value);
    const char* digit {longer + (value < 0 ? 1 : 0) + 2 + precision};
    if (*digit != '5')
    {
        return false;
    }
    for (++digit; *digit != 'e'; ++digit)
    {
        if (*digit != '0')
        {
            return false;
        }
    }
    return true;
}

static void test_examples()
{
    // The last two digits came from a 7-digit block that was cut short
    test(from_bits(UINT64_C(0x0000000000000F0E)), 15, "1.904128999072164e-320");
    test(from_bits(UINT64_C(0xD1FF07B64329EBC1)), 25, "-9.6450013237715984485328503e+86");
    test(from_bits(UINT64_C(0xB2597BD5E094B8B5)), 27, "-3.780976657374227457368861285e-66");

    // Rounding up carried through the last two digits while an odd number of digits was due
    test(from_bits(UINT64_C(0x8701CD1D005367BB)), 32, "-6.42693380817628841895686997158100e-275");
    test(from_bits(UINT64_C(0xD432B72D53567085)), 23, "-3.99761409053009441393200e+97");
}

static void test_random()
{
    std::mt19937_64 rng(42);
    for (int i {}; i < 20000; ++i)
    {
        std::uint64_t bits = rng();
        if (i % 2 == 1)
        {
            bits = (bits & ((UINT64_C(1) << 52U) - 1U)) >> (rng() % 52U);
        }
        const double value = from_bits(bits);
        if (value != value || value - value != 0 || value == 0)
        {
            continue;
        }

        for (int precision {15}; precision <= 40; ++precision)
        {
            if (is_tie(value, precision))
            {
                continue;
            }
            char expected[128];
            std::snprintf(expected, sizeof(expected), "%.*e", precision, value);
            test(value, precision, expected);
        }
    }
}

int main()
{
    test_examples();
    test_random();

    return boost::report_errors();
}
