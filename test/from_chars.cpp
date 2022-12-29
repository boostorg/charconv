// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstring>

int main()
{
    char const* buffer = "1048576";

    int v = 0;
    auto r = boost::charconv::from_chars( buffer, buffer + std::strlen( buffer ), v );

    BOOST_TEST_EQ( r.ec, 0 ) && BOOST_TEST_EQ( v, 1048576 );

    return boost::report_errors();
}
