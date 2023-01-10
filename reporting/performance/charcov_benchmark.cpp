//  (C) Copyright Matt Borland 2023.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <benchmark/benchmark.h>
#include <charconv>
#include <cstring>

template <typename T>
void std_integer_from_chars(benchmark::State& state)
{
    const char* buffer = "12345";
    T v {};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(std::from_chars(buffer, buffer + std::strlen(buffer), v));
    }
}

BENCHMARK_TEMPLATE(std_integer_from_chars, int);
BENCHMARK_TEMPLATE(std_integer_from_chars, unsigned);
BENCHMARK_TEMPLATE(std_integer_from_chars, long);
BENCHMARK_TEMPLATE(std_integer_from_chars, unsigned long);
BENCHMARK_TEMPLATE(std_integer_from_chars, long long);
BENCHMARK_TEMPLATE(std_integer_from_chars, unsigned long long);

BENCHMARK_MAIN();

/*
Apple clang version 14.0.0 (clang-1400.0.29.202)
Target: arm64-apple-darwin22.2.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin

Running ./charcov_benchmark
Run on (10 X 24.0704 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x10)
Load Average: 2.23, 1.94, 2.12
-------------------------------------------------------------------------------------
Benchmark                                           Time             CPU   Iterations
-------------------------------------------------------------------------------------
std_integer_from_chars<int>                      37.3 ns         37.3 ns     18730689
std_integer_from_chars<unsigned>                 30.8 ns         30.8 ns     22774226
std_integer_from_chars<long>                     37.5 ns         37.5 ns     18690341
std_integer_from_chars<unsigned long>            31.2 ns         31.2 ns     22543049
std_integer_from_chars<long long>                37.5 ns         37.5 ns     18708324
std_integer_from_chars<unsigned long long>       31.2 ns         31.2 ns     22555688
*/
