// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_APPLY_SIGN_HPP
#define BOOST_CHARCONV_DETAIL_APPLY_SIGN_HPP

// Workaround for warning C4146: unary minus operator applied to unsigned type, result still unsigned
// Occurs using MSVC with pre-C++17 language standards

#include <type_traits>

namespace boost { namespace charconv { namespace detail {

template <typename Integer, typename std::enable_if<std::is_signed<Integer>::value, bool>::type = true>
constexpr Integer apply_sign(Integer val) noexcept
{
    return -val;
}

template <typename Integer, typename std::enable_if<std::is_unsigned<Integer>::value, bool>::type = true>
constexpr Integer apply_sign(Integer val) noexcept
{
    return val;
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_APPLY_SIGN_HPP
