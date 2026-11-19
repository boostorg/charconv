// Copyright 2026 mzfr
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstddef>
#include <cstring>
#include <string>

template <typename T>
void test()
{
    constexpr std::size_t buffer_size = boost::charconv::limits<T>::max_chars;
    constexpr int precision = static_cast<int>(buffer_size) + 5;
    struct
    {
        char buffer[buffer_size];
        char canary[8];
    } storage;

    std::memset(&storage, '#', sizeof(storage));
    const auto general = boost::charconv::to_chars(storage.buffer, storage.buffer + buffer_size, static_cast<T>(0),
                                                   boost::charconv::chars_format::general, precision);
    BOOST_TEST(general);
    BOOST_TEST_EQ(std::string(storage.buffer, general.ptr), "0");
    for (char c : storage.canary)
    {
        BOOST_TEST_EQ(c, '#');
    }

    std::memset(&storage, '#', sizeof(storage));
    const auto fixed = boost::charconv::to_chars(storage.buffer, storage.buffer + buffer_size, static_cast<T>(0),
                                                 boost::charconv::chars_format::fixed, precision);
    BOOST_TEST(fixed.ec == std::errc::value_too_large);
    BOOST_TEST_EQ(fixed.ptr, storage.buffer + buffer_size);
    for (char c : storage.canary)
    {
        BOOST_TEST_EQ(c, '#');
    }
}

int main()
{
    #if BOOST_CHARCONV_LDBL_BITS == 80 || BOOST_CHARCONV_LDBL_BITS == 128
    test<long double>();
    #endif

    #ifdef BOOST_CHARCONV_HAS_QUADMATH
    test<__float128>();
    #endif

    #if defined(BOOST_CHARCONV_HAS_STDFLOAT128) && defined(BOOST_CHARCONV_HAS_QUADMATH)
    test<std::float128_t>();
    #endif

    return boost::report_errors();
}
