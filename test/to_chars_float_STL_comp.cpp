// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/config.hpp>

// https://en.cppreference.com/w/cpp/compiler_support/17
#if (defined(__GNUC__) && __GNUC__ >= 11) || \
    ((defined(__clang__) && __clang_major__ >= 14 && !defined(__APPLE__)) || (defined(__clang__) && defined(__APPLE__) && __clang_major__ >= 16)) || \
    (defined(_MSC_VER) && _MSC_VER >= 1924)

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <system_error>
#include <charconv>
#include <type_traits>
#include <limits>
#include <random>
#include <string>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <cerrno>

template <typename T>
void test_spot(T val, boost::charconv::chars_format fmt = boost::charconv::chars_format::general)
{
    std::chars_format stl_fmt;
    switch (fmt)
    {
    case boost::charconv::chars_format::general:
        stl_fmt = std::chars_format::general;
        break;
    case boost::charconv::chars_format::fixed:
        stl_fmt = std::chars_format::fixed;
        break;
    case boost::charconv::chars_format::scientific:
        stl_fmt = std::chars_format::scientific;
        break;
    case boost::charconv::chars_format::hex:
        stl_fmt = std::chars_format::hex;
        break;
    default:
        BOOST_UNREACHABLE_RETURN(fmt);
        break;
    }

    char buffer_boost[256];
    char buffer_stl[256];

    const auto r_boost = boost::charconv::to_chars(buffer_boost, buffer_boost + sizeof(buffer_boost), val, fmt);
    const auto r_stl = std::to_chars(buffer_stl, buffer_stl + sizeof(buffer_stl), val, stl_fmt);

    BOOST_TEST_EQ(r_boost.ec, 0);
    if (r_stl.ec != std::errc())
    {
        // STL failed
        return;
    }

    const std::ptrdiff_t diff_boost = r_boost.ptr - buffer_boost;
    const std::ptrdiff_t diff_stl = r_stl.ptr - buffer_stl;
    const auto boost_str = std::string(buffer_boost, r_boost.ptr);
    const auto stl_str = std::string(buffer_stl, r_stl.ptr);

    if (!(BOOST_TEST_CSTR_EQ(boost_str.c_str(), stl_str.c_str()) && BOOST_TEST_EQ(diff_boost, diff_stl)))
    {
        std::cerr << std::setprecision(std::numeric_limits<T>::max_digits10 + 1)
                    << "Value: " << val
                    << "\nBoost: " << boost_str.c_str()
                    << "\n  STL: " << stl_str.c_str() << std::endl;
    }
}

template <typename T>
void random_test(boost::charconv::chars_format fmt = boost::charconv::chars_format::general)
{   
    std::mt19937_64 gen(42);
    std::uniform_real_distribution<T> dist(0, std::numeric_limits<T>::max());

    for (std::size_t i = 0; i < 100'000; ++i)
    {
        test_spot(dist(gen), fmt);
    }
}

template <typename T>
void non_finite_test()
{
    test_spot(std::numeric_limits<T>::infinity());
    test_spot(-std::numeric_limits<T>::infinity());
    test_spot(std::numeric_limits<T>::quiet_NaN());

    #if (defined(__clang__) && __clang_major__ >= 16) || defined(_MSC_VER)
    //
    // Newer clang and MSVC both give the following:
    //
    // -qNaN =  -nan(ind)
    //
    test_spot(-std::numeric_limits<T>::quiet_NaN());
    #endif

    #if (defined(__clang__) && __clang_major__ >= 16)
    //
    // Newer clang also gives the following:
    //
    //  sNaN =  nan(snan)
    // -sNaN = -nan(snan)
    //
    test_spot(std::numeric_limits<T>::signaling_NaN());
    test_spot(-std::numeric_limits<T>::signaling_NaN());
    #endif
}

template <typename T>
void fixed_test()
{
    constexpr T upper_bound = std::is_same<T, double>::value ? T(std::numeric_limits<std::uint64_t>::max()) : 
                                                               T(std::numeric_limits<std::uint32_t>::max());
    
    std::mt19937_64 gen(42);
    std::uniform_real_distribution<T> dist(1, upper_bound);

    for (std::size_t i = 0; i < 1000; ++i)
    {
        test_spot(dist(gen), boost::charconv::chars_format::fixed);
    }    
}

int main()
{   
    // General format
    random_test<float>();
    random_test<double>();

    // Scientific
    random_test<float>(boost::charconv::chars_format::scientific);
    random_test<double>(boost::charconv::chars_format::scientific);

    // Hex
    random_test<float>(boost::charconv::chars_format::hex);
    random_test<double>(boost::charconv::chars_format::hex);

    // Fixed
    fixed_test<float>();
    fixed_test<double>();
    
    non_finite_test<float>();
    non_finite_test<double>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif