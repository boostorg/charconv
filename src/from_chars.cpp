// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/from_chars.hpp>

boost::charconv::from_chars_result boost::charconv::from_chars( char const* first, char const* /*last*/, int& value, int /*base*/ )
{
	value = 0;
	return { first, 0 };
}
