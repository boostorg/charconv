// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_CONCATENATE_HPP
#define BOOST_CHARCONV_DETAIL_CONCATENATE_HPP

#include <boost/charconv/config.hpp>
#include <utility>
#include <limits>
#include <cstdint>

namespace boost { namespace charconv { namespace detail {

constexpr std::uint64_t pack(std::uint32_t word1, std::uint32_t word2) noexcept
{
    return static_cast<std::uint64_t>(word1) << 32 | word2;
}

BOOST_CXX14_CONSTEXPR std::pair<std::uint32_t, std::uint32_t> unpack(std::uint64_t value)
{
    auto x = static_cast<std::uint32_t>((value & UINT64_MAX) >> UINT64_C(32));
    auto y = static_cast<std::uint32_t>(value & UINT32_MAX);

    return std::make_pair(x, y);
}

#ifdef __GLIBCXX_TYPE_INT_N_0
constexpr unsigned __int128 pack(std::uint64_t word1, std::uint64_t word2) noexcept
{
    return static_cast<unsigned __int128>(word1) << 64 | word2;
}

BOOST_CXX14_CONSTEXPR std::pair<std::uint64_t, std::uint64_t> unpack(unsigned __int128 value)
{
    auto x = static_cast<std::uint64_t>(value >> 64);
    auto y = static_cast<std::uint64_t>(value);

    return std::make_pair(x, y);
}
#endif // defined(__GLIBCXX_TYPE_INT_N_0)

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_CONCATENATE_HPP
