// Copyright 2020-2023 Daniel Lemire
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// If the architecture (e.g. ARM) does not have __int128 we need to emulate it

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/config.hpp>
#include <cstdint>

#ifndef BOOST_CHARCONV_DETAIL_EMULATED128_HPP
#define BOOST_CHARCONV_DETAIL_EMULATED128_HPP

namespace boost { namespace charconv { namespace detail {

// Emulates a __int128 using 2x 64bit ints (high and low)
struct value128
{
    alignas(BOOST_CHARCONV_HARDWARE_DESTRUCTIVE_INTERFACE_SIZE) std::uint64_t low;
    alignas(BOOST_CHARCONV_HARDWARE_DESTRUCTIVE_INTERFACE_SIZE) std::uint64_t high;
};

#ifndef BOOST_CHARCONV_HAS_INT128
// Returns the high 64 bits of the product of two 64-bit unsigned integers.
std::uint64_t umul(std::uint64_t a, std::uint64_t b) noexcept;

value128 full_multiplication(std::uint64_t v1, std::uint64_t v2) noexcept;
#else
BOOST_CHARCONV_DECL value128 full_multiplication(std::uint64_t v1, std::uint64_t v2) noexcept;
#endif

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_EMULATED128_HPP
