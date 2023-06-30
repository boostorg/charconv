// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_EMULATED256_HPP
#define BOOST_CHARCONV_DETAIL_EMULATED256_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <boost/core/bit.hpp>
#include <cstdint>

namespace boost { namespace charconv { namespace detail {

struct uint256
{
    uint128 high;
    uint128 low;

    inline friend uint256 operator>>(uint256 lhs, int amount) noexcept;
    inline uint256 &operator>>=(int amount) noexcept
    {
        *this = *this >> amount;
        return *this;
    }

    inline friend uint256 operator<<(uint256 lhs, int amount) noexcept;
    inline uint256 &operator<<=(int amount) noexcept
    {
        *this = *this << amount;
        return *this;
    }

    inline friend uint256 operator|(uint256 lhs, uint256 rhs) noexcept;
    inline uint256 &operator|=(uint256 v) noexcept
    {
        *this = *this | v;
        return *this;
    }

    inline friend bool operator==(uint256 lhs, uint256 rhs) noexcept;
    inline friend bool operator!=(uint256 lhs, uint256 rhs) noexcept;
    inline friend bool operator<(uint256 lhs, uint256 rhs) noexcept;
    inline friend bool operator<=(uint256 lhs, uint256 rhs) noexcept;
    inline friend bool operator>(uint256 lhs, uint256 rhs) noexcept;
    inline friend bool operator>=(uint256 lhs, uint256 rhs) noexcept;

    inline friend uint256 operator+(uint256 lhs, uint256 rhs) noexcept;
    inline friend uint256 operator+(uint256 lhs, uint128 rhs) noexcept;
    inline friend uint256 operator-(uint256 lhs, uint256 rhs) noexcept;
    inline uint256 &operator-=(uint256 v) noexcept;
    inline friend uint256 operator/(uint256 lhs, uint256 rhs) noexcept;
    inline friend uint256 operator/(uint256 lhs, std::uint64_t rhs) noexcept;
    inline friend uint256 operator%(uint256 lhs, uint256 rhs) noexcept;
    inline friend uint256 operator%(uint256 lhs, std::uint64_t rhs) noexcept;

private:
    inline friend int high_bit(uint256 v) noexcept;
    inline friend void div_impl(uint256 lhs, uint256 rhs, uint256& quotient, uint256& remainder) noexcept;
};

uint256 operator>>(uint256 lhs, int amount) noexcept
{
    if (amount >= 128)
    {
        return {0, lhs.high >> (amount - 128)};
    }
    else if (amount == 0)
    {
        return lhs;
    }

    return {lhs.high >> amount, (lhs.low >> amount) | (lhs.high << (128 - amount))};
}

uint256 operator<<(uint256 lhs, int amount) noexcept
{
    if (amount >= 128)
    {
        return {lhs.low << (amount - 128), 0};
    }
    else if (amount == 0)
    {
        return lhs;
    }

    return {(lhs.high << amount) | (lhs.low >> (128 - amount)), lhs.low << amount};
}

uint256 operator|(uint256 lhs, uint256 rhs) noexcept
{
    return {lhs.high | rhs.high, lhs.low | rhs.low};
}

bool operator==(uint256 lhs, uint256 rhs) noexcept
{
    return lhs.high == rhs.high && lhs.low == rhs.low;
}

bool operator!=(uint256 lhs, uint256 rhs) noexcept
{
    return !(lhs.high == rhs.high && lhs.low == rhs.low);
}

bool operator<(uint256 lhs, uint256 rhs) noexcept
{
    if (lhs.high == rhs.high)
    {
        return lhs.low < rhs.low;
    }

    return lhs.high < rhs.high;
}

bool operator<=(uint256 lhs, uint256 rhs) noexcept
{
    return !(rhs < lhs);
}

bool operator>(uint256 lhs, uint256 rhs) noexcept
{
    return rhs < lhs;
}

bool operator>=(uint256 lhs, uint256 rhs) noexcept
{
    return !(lhs < rhs);
}

uint256 operator+(uint256 lhs, uint256 rhs) noexcept
{
    const uint256 temp = {lhs.high + rhs.high, lhs.low + rhs.low};

    // Need to carry a bit into hrs
    if (temp.low < lhs.low)
    {
        return {temp.high + 1, temp.low};
    }

    return temp;
}

uint256 operator+(uint256 lhs, uint128 rhs) noexcept
{
    const uint256 temp = {lhs.high, lhs.low + rhs};

    if (temp.low < lhs.low)
    {
        return {temp.high + 1, temp.low};
    }

    return temp;
}

uint256 operator-(uint256 lhs, uint256 rhs) noexcept
{
    const uint256 temp {lhs.high - rhs.high, lhs.low - rhs.low};

    // Check for carry
    if (lhs.low < rhs.low)
    {
        return {temp.high - 1, temp.low};
    }

    return temp;
}

uint256 &uint256::operator-=(uint256 v) noexcept
{
    *this = *this - v;
    return *this;
}

uint256 operator/(uint256 lhs, uint256 rhs) noexcept
{
    uint256 quotient;
    uint256 remainder;
    div_impl(lhs, rhs, quotient, remainder);

    return quotient;
}

uint256 operator/(uint256 lhs, std::uint64_t rhs) noexcept
{
    uint256 quotient;
    uint256 remainder;
    uint256 big_rhs = {0, rhs};

    div_impl(lhs, big_rhs, quotient, remainder);

    return quotient;
}

uint256 operator%(uint256 lhs, uint256 rhs) noexcept
{
    uint256 quotient;
    uint256 remainder;
    div_impl(lhs, rhs, quotient, remainder);

    return remainder;
}

uint256 operator%(uint256 lhs, std::uint64_t rhs) noexcept
{
    uint256 quotient;
    uint256 remainder;
    uint256 big_rhs = {0, rhs};

    div_impl(lhs, big_rhs, quotient, remainder);

    return remainder;
}

int high_bit(uint256 v) noexcept
{
    if (v.high != 0)
    {
        return high_bit(v.high);
    }
    else if (v.low != 0)
    {
        return high_bit(v.low);
    }

    return 0;
}

void div_impl(uint256 lhs, uint256 rhs, uint256 &quotient, uint256 &remainder) noexcept
{
    uint256 one {0, 1};

    if (rhs > lhs)
    {
        quotient = {0, 0};
        remainder = {0, 0};
    }
    else if (lhs == rhs)
    {
        quotient = {0, 1};
        remainder = {0, 0};
    }

    uint256 denom = rhs;
    quotient = {0, 0};

    std::int32_t shift = high_bit(lhs) - high_bit(rhs);
    if (shift < 0)
    {
        shift = 32 - shift;
    }
    denom <<= shift;

    for (std::int32_t i = 0; i <= shift; ++i)
    {
        quotient <<= 1;
        if (lhs >= denom)
        {
            lhs -= denom;
            quotient |= one;
        }

        denom >>= 1;
    }

    remainder = lhs;
}

// Get the 256-bit result of multiplication of two 128-bit unsigned integers
inline uint256 umul256_impl(std::uint64_t a, std::uint64_t b, std::uint64_t c, std::uint64_t d) noexcept
{
    const auto ac = umul128(a, c);
    const auto bc = umul128(b, c);
    const auto ad = umul128(a, d);
    const auto bd = umul128(b, d);

    const auto intermediate = (bd >> 64) + static_cast<std::uint64_t>(ad) + static_cast<std::uint64_t>(bc);

    return {ac + (intermediate >> 64) + (ad >> 64) + (bc >> 64),
            (intermediate << 64) + static_cast<std::uint64_t>(bd)};
}

template <typename T>
inline uint256 umul256(const T& x, const uint128& y) noexcept
{
    static_assert(sizeof(T) == 16 && (!std::numeric_limits<T>::is_signed
                                      #ifdef BOOST_CHARCONV_HAS_INT128
                                      // May not have numeric_limits specialization without gnu mode
                                      || std::is_same<T, boost::uint128_type>::value
                                      #endif
                                      ), "This function is only for 128-bit unsigned types");

    const auto a = static_cast<std::uint64_t>(x >> 64);
    const auto b = static_cast<std::uint64_t>(x);

    return umul256_impl(a, b, y.high, y.low);
}

inline uint256 umul256(const uint128& x, const uint128& y) noexcept
{
    return umul256_impl(x.high, x.low, y.high, y.low);
}

// Returns only the high 256 bits of a 256x256 multiplication
inline uint256 umul512_high256(const uint256& x, const uint256& y) noexcept
{
    const auto a = x.high;
    const auto b = x.low;
    const auto c = y.high;
    const auto d = y.low;

    const auto ac = umul256(a, c);
    const auto bc = umul256(b, c);
    const auto ad = umul256(a, d);
    const auto bd = umul256(b, d);

    const auto intermediate = (bd >> 128) + ad.high + bc.high;

    return ac + (intermediate >> 128) + (ad >> 128) + (bc >> 128);
}



}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_EMULATED256_HPP
