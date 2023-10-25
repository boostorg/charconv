// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_APPLY_SIGN_HPP
#define BOOST_CHARCONV_DETAIL_APPLY_SIGN_HPP

#include <boost/config.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <boost/charconv/detail/type_traits.hpp>
#include <type_traits>

#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4146)
#endif

namespace boost { namespace charconv { namespace detail {

template <typename Integer, typename Unsigned_Integer = detail::make_unsigned_t<Integer>,
          typename std::enable_if<detail::is_signed<Integer>::value, bool>::type = true>
constexpr Unsigned_Integer apply_sign(Integer val) noexcept
{
    return -(static_cast<Unsigned_Integer>(val));
}

template <typename Unsigned_Integer, typename std::enable_if<!detail::is_signed<Unsigned_Integer>::value, bool>::type = true>
constexpr Unsigned_Integer apply_sign(Unsigned_Integer val) noexcept
{
    return val;
}

}}} // Namespaces

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif

#endif // BOOST_CHARCONV_DETAIL_APPLY_SIGN_HPP
