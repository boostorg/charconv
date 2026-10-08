// Copyright 2026 Pavel P
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>

static void test(const double value, const boost::charconv::chars_format fmt, const int precision, const char* expected)
{
    char buffer[128];
    const auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value, fmt, precision);
    BOOST_TEST(r);
    *r.ptr = '\0';

    BOOST_TEST_CSTR_EQ(buffer, expected);
}

// One rounding point in floff, four ways: below half, a tie after an odd digit, above half, a tie after an even digit
static void test_examples()
{
    const auto fixed = boost::charconv::chars_format::fixed;
    const auto scientific = boost::charconv::chars_format::scientific;

    test(102414767595.7294464111328125, fixed, 14, "102414767595.72944641113281");
    test(102414767595.7294464111328125, scientific, 25, "1.0241476759572944641113281e+11");
    test(137924605214.933197021484375, fixed, 14, "137924605214.93319702148438");
    test(137924605214.933197021484375, scientific, 25, "1.3792460521493319702148438e+11");
    test(76209358548.4597625732421875, fixed, 14, "76209358548.45976257324219");
    test(76209358548.4597625732421875, scientific, 24, "7.620935854845976257324219e+10");
    test(59547781175.354644775390625, fixed, 14, "59547781175.35464477539062");
    test(59547781175.354644775390625, scientific, 24, "5.954778117535464477539062e+10");
    test(-202364179515.495880126953125, fixed, 14, "-202364179515.49588012695312");
}

// A positive decimal number: all its digits, and how many of them come before the decimal point
struct decimal_digits
{
    std::string digits;
    std::size_t integer_digits;
};

static void multiply_by_5(std::string& digits)
{
    int carry {0};
    std::size_t i {digits.size()};
    while (i > 0)
    {
        --i;
        const int product {(digits[i] - '0') * 5 + carry};
        digits[i] = static_cast<char>('0' + product % 10);
        carry = product / 10;
    }
    if (carry != 0)
    {
        digits.insert(digits.begin(), static_cast<char>('0' + carry));
    }
}

// m / 2^j written out exactly, as the digits of m * 5^j with the last j after the decimal point
static decimal_digits exact_digits(const std::uint64_t m, const std::size_t j)
{
    decimal_digits number {std::to_string(m), 0U};
    for (std::size_t k {0}; k < j; ++k)
    {
        multiply_by_5(number.digits);
    }
    number.integer_digits = number.digits.size() - j;
    return number;
}

// Drops the last digit, an exact 5, and rounds half to even, which not every C runtime's printf does
static void round_off_half(decimal_digits& number)
{
    number.digits.pop_back();
    if ((number.digits.back() - '0') % 2 == 0)
    {
        return;
    }
    std::size_t i {number.digits.size()};
    while (i > 0)
    {
        --i;
        if (number.digits[i] != '9')
        {
            ++number.digits[i];
            return;
        }
        number.digits[i] = '0';
    }
    // All nines carry into a new leading digit: 99.95 rounds to 100.0
    number.digits.insert(number.digits.begin(), '1');
    ++number.integer_digits;
}

static std::string as_fixed(const decimal_digits& number)
{
    std::string text {number.digits.substr(0, number.integer_digits)};
    if (number.digits.size() > number.integer_digits)
    {
        text += '.' + number.digits.substr(number.integer_digits);
    }
    return text;
}

// The first significant digits of number in %e form; number is at least 1, so the exponent is never negative
static std::string as_scientific(const decimal_digits& number, const std::size_t significant)
{
    const std::size_t exponent {number.integer_digits - 1};
    return number.digits.substr(0, 1) + '.' + number.digits.substr(1, significant - 1) + "e+" +
           (exponent < 10 ? "0" : "") + std::to_string(exponent);
}

// m / 2^j with an odd m has exactly j fractional digits, the last one 5, so one digit shorter is an exact tie
static void test_tie(const std::uint64_t m, const int j)
{
    const double value {std::ldexp(static_cast<double>(m), -j)};

    decimal_digits expected = exact_digits(m, static_cast<std::size_t>(j));
    const std::size_t significant {expected.digits.size() - 1};
    round_off_half(expected);

    test(value, boost::charconv::chars_format::fixed, j - 1, as_fixed(expected).c_str());
    test(value, boost::charconv::chars_format::scientific, static_cast<int>(significant) - 1,
         as_scientific(expected, significant).c_str());
}

static void test_random_ties()
{
    std::mt19937_64 rng(42);
    for (int i {0}; i < 100000; ++i)
    {
        const std::uint64_t m {(rng() >> 11) | (UINT64_C(1) << 52) | 1};
        const int j {1 + static_cast<int>(rng() % 52)};
        test_tie(m, j);
    }
}

// Ties that round all nines up into a new digit need m = 2 * 10^k - 1 and j = 1, which no random m above 2^52 is
static void test_carry_ties()
{
    for (std::uint64_t m {199}; m < (UINT64_C(1) << 53); m = m * 10 + 9)
    {
        test_tie(m, 1);
    }
}

int main()
{
    test_examples();
    test_random_ties();
    test_carry_ties();

    return boost::report_errors();
}
