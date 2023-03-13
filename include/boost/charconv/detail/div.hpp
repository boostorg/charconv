// Copyright 2020-2023 Junekey Jeon
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_DIV_HPP
#define BOOST_CHARCONV_DETAIL_DIV_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/log.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <type_traits>
#include <cstdint>

namespace boost { namespace charconv { namespace detail{

// Replace n by floor(n / 10^N).
// Returns true if and only if n is divisible by 10^N.
// Precondition: n <= 10^(N+1)
// !!It takes an in-out parameter!!
template <int N>
struct divide_by_pow10_info;

template <>
struct divide_by_pow10_info<1>
{
    static constexpr std::uint32_t magic_number = 6554;
    static constexpr int           shift_amount = 16;
};

template <>
struct divide_by_pow10_info<2>
{
    static constexpr std::uint32_t magic_number = 656;
    static constexpr int           shift_amount = 16;
};

#ifdef BOOST_NO_CXX17_INLINE_VARIABLES

template <int N> constexpr std::uint32_t divide_by_pow10_info<N>::magic_number;
template <int N> constexpr int divide_by_pow10_info<N>::shift_amount;

#endif

template <int N>
BOOST_CHARCONV_CXX14_CONSTEXPR bool check_divisibility_and_divide_by_pow10(std::uint32_t& n) noexcept
{
    BOOST_CHARCONV_ASSERT(N + 1 <= log::floor_log10_pow2(31));
    BOOST_CHARCONV_ASSERT(n <= compute_power<N + 1>(UINT32_C(10)));

    using info = divide_by_pow10_info<N>;
    n *= info::magic_number;

    constexpr auto mask = static_cast<std::uint32_t>(1 << info::shift_amount) - 1;
    bool result = ((n & mask) < info::magic_number);

    n >>= info::shift_amount;

    return result;
}

// Compute floor(n / 10^N) for small n and N.
// Precondition: n <= 10^(N+1)
template <int N>
BOOST_CHARCONV_CXX14_CONSTEXPR std::uint32_t small_division_by_pow10(std::uint32_t n) noexcept 
{
    // Make sure the computation for max_n does not overflow.
    BOOST_CHARCONV_ASSERT(N + 1 <= log::floor_log10_pow2(31));
    BOOST_CHARCONV_ASSERT(n <= compute_power<N + 1>(std::uint32_t(10)));

    return (n * divide_by_pow10_info<N>::magic_number) >> divide_by_pow10_info<N>::shift_amount;
}

// Compute floor(n / 10^N) for small N.
// Precondition: n <= n_max
template <int N, typename Unsigned_Integer, Unsigned_Integer n_max>
BOOST_CHARCONV_CXX14_CONSTEXPR Unsigned_Integer divide_by_pow10(Unsigned_Integer n) noexcept 
{
    static_assert(N >= 0);

    // Specialize for 32-bit division by 100.
    // Compiler is supposed to generate the identical code for just writing
    // "n / 100", but for some reason MSVC generates an inefficient code
    // (mul + mov for no apparent reason, instead of single imul),
    // so we does this manually.
    BOOST_IF_CONSTEXPR (std::is_same<Unsigned_Integer, std::uint32_t>::value && N == 2) 
    {
        return static_cast<std::uint32_t>(umul64(n, UINT32_C(1374389535)) >> 37);
    }
    // Specialize for 64-bit division by 1000.
    // Ensure that the correctness condition is met.
    else BOOST_IF_CONSTEXPR (std::is_same<Unsigned_Integer, std::uint64_t>::value && 
                             N == 3 && n_max <= UINT64_C(15534100272597517998))
    {
        return umul128_upper64(n, UINT64_C(2361183241434822607)) >> 7;
    }
    else 
    {
        auto divisor = compute_power<N>(static_cast<Unsigned_Integer>(10));
        return n / divisor;
    }
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_DIV_HPP
