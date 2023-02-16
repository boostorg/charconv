// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/core/lightweight_test.hpp>
#include <limits>
#include <cstdint>

struct value128
{
    std::uint64_t low;
    std::uint64_t high;
};

// Returns the high 64 bits of the product of two 64-bit unsigned integers.
inline std::uint64_t umul(std::uint64_t a, std::uint64_t b) noexcept
{
    std::uint64_t    a_lo = static_cast<std::uint32_t>(a);
    std::uint64_t    a_hi = a >> 32;
    std::uint64_t    b_lo = static_cast<std::uint32_t>(b);
    std::uint64_t    b_hi = b >> 32;

    std::uint64_t    a_x_b_hi =  a_hi * b_hi;
    std::uint64_t    a_x_b_mid = a_hi * b_lo;
    std::uint64_t    b_x_a_mid = b_hi * a_lo;
    std::uint64_t    a_x_b_lo =  a_lo * b_lo;

    std::uint64_t    carry_bit = ((std::uint64_t)(std::uint32_t)a_x_b_mid + 
                                  (std::uint64_t)(std::uint32_t)b_x_a_mid + (a_x_b_lo >> 32)) >> 32;

    std::uint64_t    multhi = a_x_b_hi + (a_x_b_mid >> 32) + (b_x_a_mid >> 32) + carry_bit;

    return multhi;
}

inline value128 full_multiplication(std::uint64_t v1, std::uint64_t v2) noexcept
{
    value128 result;
    // https://developer.arm.com/documentation/dui0802/a/A64-General-Instructions/UMULH
    #ifdef __arm__
    result.high = __umulh(v1, v2);
    result.low = v1 * v2;
    #else
    result.high = umul(v1, v2);
    result.low = v1 * v2;
    #endif

    return result;
}

void test128()
{
    auto r1 = full_multiplication(1, 1);
    BOOST_TEST_EQ(r1.high, 0);
    BOOST_TEST_EQ(r1.low, 1);

    auto r2 = full_multiplication(10, std::numeric_limits<std::uint64_t>::max());
    BOOST_TEST_EQ(r2.high, 9);
    BOOST_TEST_EQ(r2.low, UINT64_C(18446744073709551606));

    auto r3 = full_multiplication(std::numeric_limits<std::uint64_t>::max(), std::numeric_limits<std::uint64_t>::max());
    BOOST_TEST_EQ(r3.high, UINT64_C(18446744073709551614));
    BOOST_TEST_EQ(r3.low, 1); 
}

int main(void)
{
    test128();
    return boost::report_errors();
}
