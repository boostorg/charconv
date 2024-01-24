// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// To use this library in header only mode include this file in exactly ONE file

#ifndef BOOST_CHARCONV_SRC_HPP
#define BOOST_CHARCONV_SRC_HPP

#ifndef _SCL_SECURE_NO_WARNINGS
# define _SCL_SECURE_NO_WARNINGS
# define BOOST_CHARCONV_DEFINED_SCL_SECURE_NO_WARNINGS
#endif

#ifndef NO_WARN_MBCS_MFC_DEPRECATION
# define NO_WARN_MBCS_MFC_DEPRECATION
# define BOOST_CHARCONV_DEFINED_NO_WARN_MBCS_MFC_DEPRECATION
#endif

#include <boost/charconv/detail/from_chars_src.hpp>
#include <boost/charconv/detail/to_chars_src.hpp>

#ifdef BOOST_CHARCONV_DEFINED_SCL_SECURE_NO_WARNINGS
# undef _SCL_SECURE_NO_WARNINGS
#endif

#ifdef BOOST_CHARCONV_DEFINED_NO_WARN_MBCS_MFC_DEPRECATION
# undef NO_WARN_MBCS_MFC_DEPRECATION
#endif

#endif //BOOST_CHARCONV_SRC_HPP
