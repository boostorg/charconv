// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/from_chars.hpp>
#include <boost/charconv/config.hpp>
#include <boost/config.hpp>
#include <type_traits>
#include <cstdlib>

template <typename Integer, typename std::enable_if<std::is_integral<Integer>::value, bool>::type>
boost::charconv::from_chars_result boost::charconv::detail::from_chars(const char* first, const char* last, Integer& value, int base)
{
    // Check pre-conditions
    BOOST_CHARCONV_ASSERT_MSG(base >= 2 && base <= 36, "Base must be between 2 and 36 (inclusive)");

    BOOST_IF_CONSTEXPR(std::is_same<Integer, int>::value)
    {
        value = static_cast<int>(std::strtol(first, const_cast<char**>(&last), base));
        return { last, 0 };
    }

    return {last, 0};
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, char& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned char& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, short& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned short& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, int& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned int& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned long& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long long& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned long long& value, int base)
{
    return boost::charconv::detail::from_chars(first, last, value, base);
}
