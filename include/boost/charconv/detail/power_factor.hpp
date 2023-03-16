// Copyright 2020-2023 Junekey Jeon
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_POWER_FACTOR_HPP
#define BOOST_CHARCONV_DETAIL_POWER_FACTOR_HPP

#include <boost/charconv/detail/config.hpp>
#include <cstdint>

namespace boost { namespace charconv { namespace detail {

template <int k, typename Integer>
BOOST_CHARCONV_CXX14_CONSTEXPR Integer compute_power(Integer a) noexcept 
{
    Integer p = 1;
    for (Integer i = 0; i < k; ++i) 
    {
        p *= a;
    }

    return p;
}

template <unsigned a, typename Unsigned_Integer>
BOOST_CHARCONV_CXX14_CONSTEXPR int count_factors(Unsigned_Integer n) noexcept 
{
    static_assert(a > 1, "a must be greater than 1");
    
    int c = 0;
    while (n % a == 0)
    {
        n /= a;
        ++c;
    }

    return c;
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_POWER_FACTOR_HPP
