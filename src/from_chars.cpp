// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// https://stackoverflow.com/questions/38060411/visual-studio-2015-wont-suppress-error-c4996
#ifndef _SCL_SECURE_NO_WARNINGS
# define _SCL_SECURE_NO_WARNINGS
#endif
#ifndef NO_WARN_MBCS_MFC_DEPRECATION
# define NO_WARN_MBCS_MFC_DEPRECATION
#endif

#include <boost/charconv/detail/from_chars_src.hpp>
