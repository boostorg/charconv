// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_MEMCPY_HPP
#define BOOST_CHARCONV_DETAIL_MEMCPY_HPP

#include <boost/charconv/detail/is_constant_evaluated.hpp>
#include <cstring>
#include <cstdint>

namespace boost { namespace charconv { namespace detail {

#if !defined(BOOST_CHARCONV_NO_CONSTEXPR_DETECTION) && defined(BOOST_CXX14_CONSTEXPR)

#define BOOST_CHARCONV_CONSTEXPR constexpr

constexpr void* memcpy(void* dest, const void* src, std::size_t count)
{
    if (BOOST_CHARCONV_IS_CONSTANT_EVALUATED(count))
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            *(dest + i) = *(src + i);
        }

        return dest;
    }
    else
    {
        return std::memcpy(dest, src, count);
    }
}

#else // Either not C++14 or no way of telling if we are in a constexpr context

#define BOOST_CHARCONV_CONSTEXPR

inline void* memcpy(void* dest, const void* src, std::size_t count)
{
    return std::memcpy(dest, src, count);
}

#endif

}}} // Namespace boost::charconv::detail

#endif // BOOST_CHARCONV_DETAIL_MEMCPY_HPP
