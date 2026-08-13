//  Copyright 2026 Matt Borland
//  Distributed under the Boost Software License, Version 1.0.
//  https://www.boost.org/LICENSE_1_0.txt

#include "sycl_charconv_test.hpp"

int main()
{
    int result {0};
    for (int base {2}; base <= 36; ++base)
    {
        result |= charconv_sycl_test::test_to_chars<unsigned int>(base);
    }
    return result;
}
