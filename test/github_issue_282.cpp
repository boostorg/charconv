// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/boostorg/charconv/issues/282

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>

template <std::size_t size>
void test_no_format()
{
    char buffer[size];
    const auto r = boost::charconv::to_chars(buffer, buffer + size, 0.1);
    if (BOOST_TEST(r))
    {
        *r.ptr = '\0';
        BOOST_TEST_CSTR_EQ(buffer, "0.1");
    }
}

template <std::size_t size>
void test_with_format()
{
    char buffer[size];
    const auto r = boost::charconv::to_chars(buffer, buffer + size, 0.1, boost::charconv::chars_format::general);
    if (BOOST_TEST(r))
    {
        *r.ptr = '\0';
        BOOST_TEST_CSTR_EQ(buffer, "0.1");
    }
}

int main()
{
    test_no_format<20>();
    test_no_format<100>();
    test_no_format<boost::charconv::limits<double>::max_chars10>();

    test_with_format<20>();
    test_with_format<100>();
    test_with_format<boost::charconv::limits<double>::max_chars10>();

    return boost::report_errors();
}
