// Copyright 2020-2023 Daniel Lemire
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// If the architecture (e.g. ARM) does not have __int128 we need to emulate it

#ifndef BOOST_CHARCONV_DETAIL_EMULATED128_HPP
#define BOOST_CHARCONV_DETAIL_EMULATED128_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/config.hpp>
#include <cstdint>
#include <cassert>

namespace boost { namespace charconv { namespace detail {

// Compilers might support built-in 128-bit integer types. However, it seems that
// emulating them with a pair of 64-bit integers actually produces a better code,
// so we avoid using those built-ins. That said, they are still useful for
// implementing 64-bit x 64-bit -> 128-bit multiplication.

class uint128 
{
public:
    uint128() = default;

    constexpr uint128(std::uint64_t high, std::uint64_t low) noexcept
        : high_{high}, low_{low} {}

    constexpr std::uint64_t high() const noexcept { return high_; }
    constexpr std::uint64_t low() const noexcept { return low_; }

    uint128& operator+=(std::uint64_t n) & noexcept 
    {
        #if BOOST_CHARCONV_HAS_BUILTIN(__builtin_addcll)
        
        unsigned long long carry;
        low_ = __builtin_addcll(low_, n, 0, &carry);
        high_ = __builtin_addcll(high_, 0, carry, &carry);
        
        #elif BOOST_CHARCONV_HAS_BUILTIN(__builtin_ia32_addcarryx_u64)
        
        unsigned long long result;
        auto carry = __builtin_ia32_addcarryx_u64(0, low_, n, &result);
        low_ = result;
        __builtin_ia32_addcarryx_u64(carry, high_, 0, &result);
        high_ = result;
        
        #elif defined(_MSC_VER) && defined(_M_X64)
        
        auto carry = _addcarry_u64(0, low_, n, &low_);
        _addcarry_u64(carry, high_, 0, &high_);
        
        #else
        
        auto sum = low_ + n;
        high_ += (sum < low_ ? 1 : 0);
        low_ = sum;
        
        #endif
        return *this;
    }

private:
    std::uint64_t high_;
    std::uint64_t low_;
};

static inline std::uint64_t umul64(std::uint32_t x, std::uint32_t y) noexcept 
{
#if defined(BOOST_CHARCONV_HAS_MSVC_32BIT_INTRINSICS)
    return __emulu(x, y);
#else
    return x * static_cast<std::uint64_t>(y);
#endif
}

// Get 128-bit result of multiplication of two 64-bit unsigned integers.
BOOST_CHARCONV_SAFEBUFFERS inline uint128 umul128(std::uint64_t x, std::uint64_t y) noexcept 
{
    #if defined(BOOST_CHARCONV_HAS_INT128)
    
    auto result = static_cast<boost::uint128_type>(x) * static_cast<boost::uint128_type>(y);
    return {static_cast<std::uint64_t>(result >> 64), static_cast<std::uint64_t>(result)};
    
    #elif defined(BOOST_CHARCONV_HAS_MSVC_64BIT_INTRINSICS)
    
    std::uint64_t high;
    std::uint64_t low = _umul128(x, y, &high);
    return {high, low};
    
    // https://developer.arm.com/documentation/dui0802/a/A64-General-Instructions/UMULH
    #elif defined(__arm__)

    std::uint64_t high = __umulh(x, y);
    std::uint64_t low = x * y;
    return {high, low};

    #else
    
    auto a = static_cast<std::uint32_t>(x >> 32);
    auto b = static_cast<std::uint32_t>(x);
    auto c = static_cast<std::uint32_t>(y >> 32);
    auto d = static_cast<std::uint32_t>(y);

    auto ac = umul64(a, c);
    auto bc = umul64(b, c);
    auto ad = umul64(a, d);
    auto bd = umul64(b, d);

    auto intermediate = (bd >> 32) + static_cast<std::uint32_t>(ad) + static_cast<std::uint32_t>(bc);

    return {ac + (intermediate >> 32) + (ad >> 32) + (bc >> 32),
            (intermediate << 32) + static_cast<std::uint32_t>(bd)};
    
    #endif
}

BOOST_CHARCONV_SAFEBUFFERS inline std::uint64_t umul128_upper64(std::uint64_t x, std::uint64_t y) noexcept
{
    #if defined(BOOST_CHARCONV_HAS_INT128)
    
    auto result = static_cast<boost::uint128_type>(x) * static_cast<boost::uint128_type>(y);
    return static_cast<std::uint64_t>(result >> 64);
    
    #elif defined(BOOST_CHARCONV_HAS_MSVC_64BIT_INTRINSICS)
    
    return __umulh(x, y);
    
    #else
    
    auto a = static_cast<std::uint32_t>(x >> 32);
    auto b = static_cast<std::uint32_t>(x);
    auto c = static_cast<std::uint32_t>(y >> 32);
    auto d = static_cast<std::uint32_t>(y);

    auto ac = umul64(a, c);
    auto bc = umul64(b, c);
    auto ad = umul64(a, d);
    auto bd = umul64(b, d);

    auto intermediate = (bd >> 32) + static_cast<std::uint32_t>(ad) + static_cast<std::uint32_t>(bc);

    return ac + (intermediate >> 32) + (ad >> 32) + (bc >> 32);
    
    #endif
}

// Get upper 128-bits of multiplication of a 64-bit unsigned integer and a 128-bit
// unsigned integer.
BOOST_CHARCONV_SAFEBUFFERS inline uint128 umul192_upper128(std::uint64_t x, uint128 y) noexcept
{
    auto r = umul128(x, y.high());
    r += umul128_upper64(x, y.low());
    return r;
}

// Get upper 64-bits of multiplication of a 32-bit unsigned integer and a 64-bit
// unsigned integer.
inline std::uint64_t umul96_upper64(std::uint32_t x, std::uint64_t y) noexcept 
{
    #if defined(BOOST_CHARCONV_HAS_INT128) || defined(BOOST_CHARCONV_HAS_MSVC_64BIT_INTRINSICS)
    
    return umul128_upper64(static_cast<std::uint64_t>(x) << 32, y);
    
    #else
    
    auto yh = static_cast<std::uint32_t>(y >> 32);
    auto yl = static_cast<std::uint32_t>(y);

    auto xyh = umul64(x, yh);
    auto xyl = umul64(x, yl);

    return xyh + (xyl >> 32);

    #endif
}

// Get lower 128-bits of multiplication of a 64-bit unsigned integer and a 128-bit
// unsigned integer.
BOOST_CHARCONV_SAFEBUFFERS inline uint128 umul192_lower128(std::uint64_t x, uint128 y) noexcept
{
    auto high = x * y.high();
    auto high_low = umul128(x, y.low());
    return {high + high_low.high(), high_low.low()};
}

// Get lower 64-bits of multiplication of a 32-bit unsigned integer and a 64-bit
// unsigned integer.
inline std::uint64_t umul96_lower64(std::uint32_t x, std::uint64_t y) noexcept 
{
    return x * y;
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_EMULATED128_HPP
