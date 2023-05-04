// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/boostorg/json/issues/599

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>

void grind_double(const std::string& str, const double expected_value)
{
    // From string to expected value
    double v {};
    auto from_r = boost::charconv::from_chars(str.c_str(), str.c_str() + std::strlen(str.c_str()), v);
    if (!(BOOST_TEST_EQ(v, expected_value) && BOOST_TEST_EQ(from_r.ec, 0)))
    {
        std::cerr << "Expected value: " << expected_value << "\nFrom chars value: " << v << std::endl;
        return;
    }

    // Roundtrip
    double roundtrip_v {};
    char buffer[256] {};
    auto to_r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), expected_value);
    BOOST_TEST_EQ(to_r.ec, 0);

    auto roundtrip_r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), roundtrip_v);
    if (!(BOOST_TEST_EQ(roundtrip_v, expected_value) && BOOST_TEST_EQ(roundtrip_r.ec, 0)))
    {
        std::cerr << "Expected value: " << expected_value << "\nRoundtrip value: " << roundtrip_v << std::endl;
        return;
    }
}

// https://github.com/boostorg/json/issues/599
void issue_599_test()
{
    const std::vector<double> ref_values = {
            -0.27006296145688152
            , -0.42448451824686562
            , 0.057166336253381224
            , 1217.2772861138403
            , -161.68713249779881
            , 267.04251495962637
            , -0.66615716744854903
            , -0.80918535242172396
            , 0.29123256908034584
            , 2137.0241926849581
            , -599.61476423470071
            , 1002.9111801605201
            , -1.4239725866123634
            , -1.0346132515963697
            , 0.90790798114618365
            , 2535.2404973980229
            , -1207.1290929173115
            , 2028.379668845469
            , -2.2996584528580817
            , -0.90521880279912548
            , 1.6449616573025234
            , 2314.9160231072947
            , -1836.2705293282695
            , 3093.6759124836558
            , -2.7781953227473752
            , -0.54944860097807424
            , 1.9702410871568496
            , 1529.7366713650281
            , -2333.8061352785221
            , 3939.3529821428001
            , -3.0696620243882053
            , -0.13139419714747352
            , 2.0689802496328444
            , 370.91535570754445
            , -2578.5523049665962
            , 4359.4347976947429
            , 2.9538186832340876
            , 0.29606564516163103
            , 2.0456322162820761
            , -879.28776788268817
            , -2510.8913760620435
            , 4251.6098535812462 };

    for (const auto current_ref_val : ref_values)
    {
        char buffer[256] {};
        const auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), current_ref_val);
        BOOST_TEST_EQ(r.ec, 0);

        double return_val {};
        const auto return_r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), return_val);
        BOOST_TEST_EQ(return_r.ec, 0);
        if (!BOOST_TEST_EQ(current_ref_val, return_val))
        {
            #ifdef BOOST_CHARCONV_DEBUG
            std::cerr << std::setprecision(17)
                  << "     Value: " << current_ref_val
                  << "\n  To chars: " << std::string( buffer, r.ptr )
                  << "\nFrom chars: " << return_val << std::endl;
            #else
            std::cerr << "... test failure for value=" << current_ref_val << "; buffer='" << std::string( buffer, r.ptr ) << "'" << std::endl;
            #endif
        }
    }
}

int main()
{
    issue_599_test();

    grind_double("-1.010", -1.01);
    grind_double("-0.010", -0.01);
    grind_double("-0.0", -0.0);
    grind_double("-0e0", -0.0);
    grind_double("18.4",  18.4);
    grind_double("-18.4", -18.4);
    grind_double("18446744073709551616",  1.8446744073709552e+19);
    grind_double("-18446744073709551616", -1.8446744073709552e+19);
    grind_double("18446744073709551616.0",  1.8446744073709552e+19);
    grind_double("18446744073709551616.00009",  1.8446744073709552e+19);
    grind_double("1844674407370955161600000",  1.8446744073709552e+24);
    grind_double("-1844674407370955161600000", -1.8446744073709552e+24);
    grind_double("1844674407370955161600000.0",  1.8446744073709552e+24);
    grind_double("1844674407370955161600000.00009",  1.8446744073709552e+24);
    grind_double("19700720435664.186294290058937593e13",  1.9700720435664185e+26);

    grind_double("1.0", 1.0);
    grind_double("1.1", 1.1);
    grind_double("1.11", 1.11);
    grind_double("1.11111", 1.11111);
    grind_double("11.1111", 11.1111);
    grind_double("111.111", 111.111);

    return boost::report_errors();
}
