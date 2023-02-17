// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_LEADING_ZEROS_HPP
#define BOOST_CHARCONV_DETAIL_LEADING_ZEROS_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/integer_search_trees.hpp>
#include <cstdint>
#include <limits>

namespace boost { namespace charconv { namespace detail {

static constexpr int index64[] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

// https://www.chessprogramming.org/BitScan#Bitscan_reverse
inline int bitscan_reverse(std::uint64_t bb) noexcept
{
   constexpr std::uint64_t debruijn64 = UINT64_C(0x03f79d71b4cb0a89);
   bb |= bb >> 1; 
   bb |= bb >> 2;
   bb |= bb >> 4;
   bb |= bb >> 8;
   bb |= bb >> 16;
   bb |= bb >> 32;

   return index64[(bb * debruijn64) >> 58];
}

// Search the mask data from most significant bit (MSB) to least significant bit (LSB) for a set bit (1).
// Use assembly, intrinsics, or fallback to the above if we have nothing else
inline int leading_zeros(std::uint64_t val) noexcept
{
    if (val == 0)
    {
        return 63;
    }
#if defined(BOOST_CHARCONV_HAS_MSVC_64BIT_INTRINSICS)
    unsigned long leading_zero;
    if (_BitScanReverse64(&leading_zero, val))
    {
        return static_cast<int>(63 - leading_zero);
    }
#elif defined(BOOST_CHARCONV_HAS_MSVC_32BIT_INTRINSICS)
    unsigned long leading_zero;
    if (_BitScanReverse(&leading_zero, static_cast<std::uint32_t>(val >> 32)))
    {
        return static_cast<int>(63 - (leading_zero + 32));
    }
    if (_BitScanReverse(&leading_zero, static_cast<std::uint32_t>(val)))
    {
        return static_cast<int>(63 - leading_zero);
    }
#elif defined(BOOST_CHARCONV_HAS_X86_INTRINSICS)
    return static_cast<int>(_lzcnt_u64(val);)
#elif defined(BOOST_CHARCONV_HAS_ARM_INTRINSICS)
    // CLZ Xd, Xm
    return __builtin_clzll(val);
#elif defined(BOOST_CHARCONV_HAS_NO_INTRINSICS)
    return static_cast<int>(63 - bitscan_reverse(val));
#endif
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_LEADING_ZEROS_HPP
