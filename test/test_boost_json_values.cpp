// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/boostorg/json/issues/599

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <type_traits>
#include <limits>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cerrno>
#include <utility>

int main()
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
        // Currently boost.json converts double values to scientific by default
        char buffer[256];
        const auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), current_ref_val, boost::charconv::chars_format::scientific);
        BOOST_TEST_EQ(r.ec, 0);

        // Roundtrip for scientifc representation
        double return_val;
        const auto return_r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), return_val);
        BOOST_TEST_EQ(return_r.ec, 0);
        BOOST_TEST_EQ(current_ref_val, return_val);
    }

    return boost::report_errors();
}
