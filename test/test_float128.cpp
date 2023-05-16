// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <cmath>

#ifdef BOOST_CHARCONV_HAS_FLOAT128

template <typename T>
void spot_value(const std::string& buffer, T expected_value, boost::charconv::chars_format fmt = boost::charconv::chars_format::general)
{
    T v = 0;
    auto r = boost::charconv::from_chars(buffer.c_str(), buffer.c_str() + std::strlen(buffer.c_str()), v, fmt);
    BOOST_TEST_EQ(r.ec, 0);
    if (!BOOST_TEST(v == expected_value))
    {
        char buf[64] {};
        quadmath_snprintf(buf, sizeof(buf), "%.46Qg", v);
        std::cerr << "Test failure for: " << buffer << " got: " << buf << std::endl;
    }
}

int main()
{
    spot_value("-1.010", -1.01Q);
    spot_value("-0.010", -0.01Q);
    spot_value("-0.0", -0.0Q);
    spot_value("-0e0", -0.0Q);
    spot_value( "18.4",  18.4Q);
    spot_value("-18.4", -18.4Q);

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
