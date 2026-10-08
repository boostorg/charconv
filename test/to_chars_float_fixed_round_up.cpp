// Copyright 2026 Pavel P
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cmath>
#include <cstdio>

static void test(const double value, const int precision, const char* expected)
{
    char buffer[128];
    const auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value,
                                             boost::charconv::chars_format::fixed, precision);
    BOOST_TEST(r);
    *r.ptr = '\0';

    BOOST_TEST_CSTR_EQ(buffer, expected);
}

static void test_examples()
{
    test(0.0099995, 4, "0.0100");
    test(-0.0099995, 4, "-0.0100");
    test(0.099999, 3, "0.100");
    test(0.00099999, 5, "0.00100");
    test(0.999999, 3, "1.000");
}

// Values just below 10^-k, at every precision where rounding up reaches 10^-k
static void test_below_powers_of_ten()
{
    for (int k {1}; k <= 15; ++k)
    {
        for (int m {1}; m <= 15; ++m)
        {
            const double value {std::pow(10.0, -k) - 0.3 * std::pow(10.0, -k - m)};
            for (int precision {k}; precision <= k + 20; ++precision)
            {
                char expected[128];
                std::snprintf(expected, sizeof(expected), "%.*f", precision, value);
                test(value, precision, expected);
            }
        }
    }
}

int main()
{
    test_examples();
    test_below_powers_of_ten();

    return boost::report_errors();
}
