// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    std::string s {reinterpret_cast<const char*>(data), size};

    try
    {
        long val;
        std::string s {reinterpret_cast<const char*>(data), size};
        boost::charconv::from_chars(s.c_str(), s.c_str() + s.size(), val);
    }
    catch(...)
    {
        std::cerr << "Error with: " << s << std::endl;
    }

    return 0;
}
