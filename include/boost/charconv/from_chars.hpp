// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED

#include <boost/charconv/config.hpp>
#include <type_traits>
#include <array>
#include <cstdint>

namespace boost { namespace charconv {

// 22.13.3, Primitive numerical input conversion

struct from_chars_result
{
    const char* ptr;

    // Values:
    // 0 = no error
    // 1 = invalid_argument
    // 2 = result_out_of_range
    int ec;

    friend bool operator==(const from_chars_result& lhs, const from_chars_result& rhs)
    {
        return lhs.ptr == rhs.ptr && lhs.ec == rhs.ec;
    }

    friend bool operator!=(const from_chars_result& lhs, const from_chars_result& rhs)
    {
        return !(lhs == rhs);
    }
};

namespace detail {

static constexpr std::array<unsigned char, 256> uchar_values =
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
       0,   1,   2,   3,   4,   5,   6,   7,   8,   9, 255, 255, 255, 255, 255, 255,
     255,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
      25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35, 255, 255, 255, 255, 255,
     255,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
      25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

// Convert characters for 0-9, A-Z, a-z to 0-35. Anything else is 255
unsigned char digit_from_char(char val)
{
    return uchar_values[static_cast<std::size_t>(val)];
}

template <typename Integer, typename std::enable_if<std::is_integral<Integer>::value, bool>::type = true>
boost::charconv::from_chars_result from_chars(const char* first, const char* last, Integer& value, int base);

} // Namespace detail

BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, char& value, int base = 10);
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, unsigned char& value, int base = 10);
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, short& value, int base = 10);
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, unsigned short& value, int base = 10);
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, int& value, int base = 10);
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, unsigned int& value, int base = 10);
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, long& value, int base = 10);
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, unsigned long& value, int base = 10);
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, long long& value, int base = 10);
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, unsigned long long& value, int base = 10);

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
