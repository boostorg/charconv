//  Copyright 2026 Matt Borland
//  Distributed under the Boost Software License, Version 1.0.
//  https://www.boost.org/LICENSE_1_0.txt

#include "sycl_charconv_test.hpp"

int main()
{
    return charconv_sycl_test::test_from_chars<int>();
}
