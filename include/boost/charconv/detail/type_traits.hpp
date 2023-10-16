// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_TYPE_TRAITS_HPP
#define BOOST_CHARCONV_DETAIL_TYPE_TRAITS_HPP

#include <boost/charconv/detail/config.hpp>
#include <type_traits>

namespace boost { namespace charconv { namespace detail {

template <typename T>
struct is_signed { static constexpr bool value = std::is_signed<T>::value; };

#ifdef BOOST_CHARCONV_HAS_INT128

template <>
struct is_signed<boost::int128_type> { static constexpr bool value = true; };

template <>
struct is_signed<boost::uint128_type> { static constexpr bool value = false; };

#endif

#if defined(BOOST_NO_CXX17_INLINE_VARIABLES) && BOOST_MSVC != 1900

template <typename T>
constexpr bool is_signed<T>::value;

#endif

}}} // Namespaces

#endif //BOOST_CHARCONV_DETAIL_TYPE_TRAITS_HPP
