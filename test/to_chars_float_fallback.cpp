// Copyright 2018 Ulf Adams
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifdef BOOST_USE_MODULES
import std;
import boost.core;
#include <boost/core/lightweight_test_macros.hpp>
#include <boost/charconv/detail/global_module_fragment.hpp>
#else
#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstdio>
#endif

#include <boost/charconv/detail/private/fallback_routines.hpp>


constexpr const char* fmt_general(double)
{
    return "%g";
}

constexpr const char* fmt_general(long double)
{
    return "%Lg";
}

constexpr const char* fmt_sci(double)
{
    return "%e";
}

constexpr const char* fmt_sci(long double)
{
    return "%Le";
}

constexpr const char* fmt_fixed(double)
{
    return "%.0f";
}

constexpr const char* fmt_fixed(long double)
{
    return "%.0Lf";
}

template <typename T>
void test_printf_fallback(T v, const std::string&, boost::charconv::chars_format fmt = boost::charconv::chars_format::general, int precision = -1)
{
    char buffer[256] {};
    const auto r = boost::charconv::detail::to_chars_printf_impl(buffer, buffer + sizeof(buffer), v, fmt, precision);
    BOOST_TEST(r.ec == std::errc());

    char printf_buffer[256] {};
    if (fmt == boost::charconv::chars_format::general)
    {
        std::snprintf(printf_buffer, sizeof(printf_buffer), fmt_general(v), v);
    }
    else if (fmt == boost::charconv::chars_format::scientific)
    {
        std::snprintf(printf_buffer, sizeof(printf_buffer), fmt_sci(v), v);
    }
    else if (fmt == boost::charconv::chars_format::fixed)
    {
        std::snprintf(printf_buffer, sizeof(printf_buffer), fmt_fixed(v), v);
    }

    BOOST_TEST_CSTR_EQ(buffer, printf_buffer);
}

int main()
{
    test_printf_fallback(1.0, "1");
    test_printf_fallback(1.2, "1.2");
    test_printf_fallback(1.23, "1.23");
    test_printf_fallback(1.234, "1.234");
    test_printf_fallback(1.2345, "1.2345");
    test_printf_fallback(1.23456, "1.23456");
    test_printf_fallback(1.234567, "1.234567");
    test_printf_fallback(1.2345678, "1.2345678");
    test_printf_fallback(1.23456789, "1.23456789");
    test_printf_fallback(1.234567890, "1.23456789");
    test_printf_fallback(1.2345678901, "1.2345678901");
    test_printf_fallback(1.23456789012, "1.23456789012");
    test_printf_fallback(1.234567890123, "1.234567890123");
    test_printf_fallback(1.2345678901234, "1.2345678901234");
    test_printf_fallback(1.23456789012345, "1.23456789012345");
    test_printf_fallback(1.234567890123456, "1.234567890123456");

    test_printf_fallback(1.0, "1e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2, "1.2e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23, "1.23e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234, "1.234e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2345, "1.2345e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23456, "1.23456e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234567, "1.234567e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2345678, "1.2345678e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23456789, "1.23456789e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234567890, "1.23456789e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2345678901, "1.2345678901e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23456789012, "1.23456789012e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234567890123, "1.234567890123e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2345678901234, "1.2345678901234e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23456789012345, "1.23456789012345e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234567890123456, "1.234567890123456e+00", boost::charconv::chars_format::scientific);

    test_printf_fallback(1.0, "1e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2, "1.2e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23, "1.23e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234, "1.234e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2345, "1.2345e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23456, "1.23456e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234567, "1.234567e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2345678, "1.2345678e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23456789, "1.23456789e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234567890, "1.23456789e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2345678901, "1.2345678901e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23456789012, "1.23456789012e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234567890123, "1.234567890123e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2345678901234, "1.2345678901234e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23456789012345, "1.23456789012345e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234567890123456, "1.234567890123456e+00", boost::charconv::chars_format::fixed);

    test_printf_fallback(1.0L, "1");
    test_printf_fallback(1.2L, "1.2");
    test_printf_fallback(1.23L, "1.23");
    test_printf_fallback(1.234L, "1.234");
    test_printf_fallback(1.2345L, "1.2345");
    test_printf_fallback(1.23456L, "1.23456");
    test_printf_fallback(1.234567L, "1.234567");
    test_printf_fallback(1.2345678L, "1.2345678");
    test_printf_fallback(1.23456789L, "1.23456789");
    test_printf_fallback(1.234567890L, "1.23456789");
    test_printf_fallback(1.2345678901L, "1.2345678901");
    test_printf_fallback(1.23456789012L, "1.23456789012");
    test_printf_fallback(1.234567890123L, "1.234567890123");
    test_printf_fallback(1.2345678901234L, "1.2345678901234");
    test_printf_fallback(1.23456789012345L, "1.23456789012345");
    test_printf_fallback(1.234567890123456L, "1.234567890123456");

    test_printf_fallback(1.0L, "1e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2L, "1.2e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23L, "1.23e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234L, "1.234e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2345L, "1.2345e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23456L, "1.23456e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234567L, "1.234567e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2345678L, "1.2345678e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23456789L, "1.23456789e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234567890L, "1.23456789e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2345678901L, "1.2345678901e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23456789012L, "1.23456789012e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234567890123L, "1.234567890123e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.2345678901234L, "1.2345678901234e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.23456789012345L, "1.23456789012345e+00", boost::charconv::chars_format::scientific);
    test_printf_fallback(1.234567890123456L, "1.234567890123456e+00", boost::charconv::chars_format::scientific);

    test_printf_fallback(1.0L, "1e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2L, "1.2e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23L, "1.23e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234L, "1.234e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2345L, "1.2345e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23456L, "1.23456e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234567L, "1.234567e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2345678L, "1.2345678e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23456789L, "1.23456789e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234567890L, "1.23456789e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2345678901L, "1.2345678901e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23456789012L, "1.23456789012e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234567890123L, "1.234567890123e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.2345678901234L, "1.2345678901234e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.23456789012345L, "1.23456789012345e+00", boost::charconv::chars_format::fixed);
    test_printf_fallback(1.234567890123456L, "1.234567890123456e+00", boost::charconv::chars_format::fixed);

    return boost::report_errors();
}
