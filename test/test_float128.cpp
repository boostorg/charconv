// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/detail/config.hpp>

#ifdef BOOST_CHARCONV_HAS_FLOAT128

#ifdef BOOST_CHARCONV_HAS_STDFLOAT128
#define BOOST_CHARCONV_HAS_FLOAT128_OSTREAM
#include <ostream>
#include <charconv>

std::ostream& operator<<( std::ostream& os, __float128 v )
{
    char buffer[ 256 ] {};
    std::to_chars(buffer, buffer + sizeof(buffer), static_cast<std::float128_t>(v));
    os << buffer;
    return os;
}

#endif // BOOST_CHARCONV_HAS_STDFLOAT128

#include <boost/charconv/detail/issignaling.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>

void test_signaling_nan()
{
    #if BOOST_CHARCONV_HAS_BUILTIN(__builtin_nansq)
    BOOST_TEST(boost::charconv::detail::issignaling(__builtin_nansq("")));
    BOOST_TEST(boost::charconv::detail::issignaling(-__builtin_nansq("")));
    #endif

    BOOST_TEST(!boost::charconv::detail::issignaling(std::numeric_limits<__float128>::quiet_NaN()));
    BOOST_TEST(!boost::charconv::detail::issignaling(std::numeric_limits<__float128>::infinity()));
    BOOST_TEST(!boost::charconv::detail::issignaling(-std::numeric_limits<__float128>::quiet_NaN()));
    BOOST_TEST(!boost::charconv::detail::issignaling(-std::numeric_limits<__float128>::infinity()));
}

int main()
{
    test_signaling_nan();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
