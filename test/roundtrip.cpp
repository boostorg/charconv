// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstring>

static void test_roundtrip( int value, int base )
{
    char buffer[ 65 ] = {};

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ) - 1, value, base );

    BOOST_TEST_EQ( r.ec, 0 );

    int v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2, base );

    BOOST_TEST_EQ( r2.ec, 0 ) && BOOST_TEST_EQ( v2, value );
}

int main()
{
    test_roundtrip( 1048576, 10 );
    return boost::report_errors();
}
