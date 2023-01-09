// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstring>

int main()
{
    char buffer[ 32 ] = {};

    int v = 1048576;
    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ) - 1, v );

    BOOST_TEST(!std::make_error_code(r.ec));
    BOOST_TEST_CSTR_EQ(buffer, "1048576");

    return boost::report_errors();
}
