// Copyright 2024 Alexander Grund
// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <locale>
#include <boost/charconv.hpp>
#include <boost/charconv/detail/from_chars_float_impl.hpp>
#include <boost/core/lightweight_test.hpp>

template <typename T>
void test()
{
    const char buffer[] = "1.1897e+2";
    std::locale::global(std::locale("de_DE.UTF-8"));

    T v = 0;
    auto r = boost::charconv::detail::from_chars_strtod(buffer, buffer + sizeof(buffer), v);
    BOOST_TEST(r);

    std::locale::global(std::locale::classic());
    T v2 = 0;
    auto r2 = boost::charconv::detail::from_chars_strtod(buffer, buffer + sizeof(buffer), v2);
    BOOST_TEST(r2);

    BOOST_TEST_EQ(v, v2);
    BOOST_TEST(r.ptr == r2.ptr);
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();

    #ifdef BOOST_CHARCONV_HAS_FLOAT128
    test<__float128>();
    #endif

    return boost::report_errors();
}
