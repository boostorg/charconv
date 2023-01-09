// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstring>

int main()
{
    const char* buffer = "1048576";

    int v = 0;
    auto r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), v);

    BOOST_TEST_EQ( r.ec, 0 ) && BOOST_TEST_EQ( v, 1048576 );
    BOOST_TEST(r == r);

    boost::charconv::from_chars_result r2 {r.ptr, 0};
    BOOST_TEST(r == r2);

    const char* buffer2 = "1234567";
    int v2 = 0;
    auto r3 = boost::charconv::from_chars(buffer2, buffer2 + std::strlen(buffer), v2);
    BOOST_TEST(r != r3);

    return boost::report_errors();
}
