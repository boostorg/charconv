#ifndef BOOST_CHARCONV_CONFIG_HPP_INCLUDED
#define BOOST_CHARCONV_CONFIG_HPP_INCLUDED

// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/config.hpp>

// This header implements separate compilation features as described in
// http://www.boost.org/more/separate_compilation.html

#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_CHARCONV_DYN_LINK)
# if defined(BOOST_CHARCONV_SOURCE)
#  define BOOST_CHARCONV_DECL BOOST_SYMBOL_EXPORT
# else
#  define BOOST_CHARCONV_DECL BOOST_SYMBOL_IMPORT
# endif
#else
# define BOOST_CHARCONV_DECL
#endif

// Autolink

#if !defined(BOOST_CHARCONV_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_CHARCONV_NO_LIB)

#define BOOST_LIB_NAME boost_charconv

#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_CHARCONV_DYN_LINK)
# define BOOST_DYN_LINK
#endif

#include <boost/config/auto_link.hpp>

#endif

#ifndef BOOST_CHARCONV_STANDALONE
#  include <boost/assert.hpp>
#  define BOOST_CHARCONV_ASSERT(expr) BOOST_ASSERT(expr)
#  define BOOST_CHARCONV_ASSERT_MSG(expr, msg) BOOST_ASSERT_MSG(expr, msg)
#else // Use plain asserts
#  include <cassert>
#  define BOOST_CHARCONV_ASSERT(expr) assert(expr)
#  define BOOST_CHARCONV_ASSERT_MSG(expr, msg) assert((expr)&&(msg))
#endif

// Use 128 bit integers and supress warnings for using extensions
#if defined(BOOST_HAS_INT128) && !defined(__STRICT_ANSI__)
#  define BOOST_CHARCONV_HAS_INT128
#endif

#endif // BOOST_CHARCONV_CONFIG_HPP_INCLUDED
