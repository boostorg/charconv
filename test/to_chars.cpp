// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstring>

int main()
{
    char buffer[32] = {};

    int v = 1048576;
    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ) - 1, v );

    BOOST_TEST_EQ(r.ec, 0) && BOOST_TEST_CSTR_EQ(buffer, "1048576");
    BOOST_TEST(r == r);

    boost::charconv::to_chars_result r2 {r.ptr, 0};
    BOOST_TEST(r == r2);

    char buffer2[32] = {};

    auto r3 = boost::charconv::to_chars(buffer2, buffer2 + sizeof(buffer) - 1, v);
    BOOST_TEST(r != r3);

    return boost::report_errors();
}
