//  Copyright 2026 Matt Borland
//  Distributed under the Boost Software License, Version 1.0.
//  https://www.boost.org/LICENSE_1_0.txt
//
//  Shared harness for the Boost.Charconv SYCL tests. Each test runs to_chars/from_chars
//  element-wise on the SYCL device over random inputs and verifies the device results
//  against a host recomputation of the same operation.

#ifndef BOOST_CHARCONV_TEST_SYCL_CHARCONV_TEST_HPP
#define BOOST_CHARCONV_TEST_SYCL_CHARCONV_TEST_HPP

#include <sycl/sycl.hpp>
#include <boost/charconv.hpp>
#include <boost/charconv/detail/integer_search_trees.hpp>
#include <iostream>
#include <random>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace charconv_sycl_test {

constexpr int num_elements {20000};
// Large enough for the widest built-in integer in base 2 (64 digits) plus a sign.
constexpr int buf_size {128};

// to_chars: format each value on the device and compare against the host.
template <typename T>
int test_to_chars(const int base = 10)
{
    sycl::queue q;
    std::cout << "SYCL device: " << q.get_device().get_info<sycl::info::device::name>() << "\n";

    T* in {sycl::malloc_shared<T>(num_elements, q)};
    char* out_str {sycl::malloc_shared<char>(num_elements * buf_size, q)};
    int* out_len {sycl::malloc_shared<int>(num_elements, q)};

    std::mt19937_64 rng {42};
    for (int i {0}; i < num_elements; ++i)
    {
        in[i] = static_cast<T>(rng());
    }

    q.submit([&](sycl::handler& h)
    {
        h.parallel_for(sycl::range<1>(num_elements), [=](sycl::id<1> idx)
        {
            const int i {static_cast<int>(idx[0])};
            char* buf {out_str + i * buf_size};
            const auto r {boost::charconv::to_chars(buf, buf + buf_size, in[i], base)};
            out_len[i] = static_cast<int>(r.ptr - buf);
        });
    }).wait();

    int failures {0};
    for (int i {0}; i < num_elements; ++i)
    {
        char buf[buf_size];
        const auto r {boost::charconv::to_chars(buf, buf + buf_size, in[i], base)};
        const int n {static_cast<int>(r.ptr - buf)};
        if (out_len[i] != n || std::memcmp(out_str + i * buf_size, buf, static_cast<std::size_t>(n)) != 0)
        {
            if (failures < 5)
            {
                std::cerr << "Mismatch at element " << i << " (base " << base << ")\n";
            }
            ++failures;
        }
    }

    sycl::free(in, q);
    sycl::free(out_str, q);
    sycl::free(out_len, q);

    if (failures == 0)
    {
        std::cout << "Test PASSED\n";
        return EXIT_SUCCESS;
    }

    std::cerr << "Test FAILED with " << failures << " mismatches\n";
    return EXIT_FAILURE;
}

// from_chars: format each value on the host, parse it back on the device, verify round trip.
template <typename T>
int test_from_chars(const int base = 10)
{
    sycl::queue q;
    std::cout << "SYCL device: " << q.get_device().get_info<sycl::info::device::name>() << "\n";

    char* in_str {sycl::malloc_shared<char>(num_elements * buf_size, q)};
    int* in_len {sycl::malloc_shared<int>(num_elements, q)};
    T* out {sycl::malloc_shared<T>(num_elements, q)};
    T* expected {sycl::malloc_shared<T>(num_elements, q)};

    std::mt19937_64 rng {42};
    for (int i {0}; i < num_elements; ++i)
    {
        expected[i] = static_cast<T>(rng());
        char buf[buf_size];
        const auto r {boost::charconv::to_chars(buf, buf + buf_size, expected[i], base)};
        const int n {static_cast<int>(r.ptr - buf)};
        std::memcpy(in_str + i * buf_size, buf, static_cast<std::size_t>(n));
        in_len[i] = n;
    }

    q.submit([&](sycl::handler& h)
    {
        h.parallel_for(sycl::range<1>(num_elements), [=](sycl::id<1> idx)
        {
            const int i {static_cast<int>(idx[0])};
            const char* buf {in_str + i * buf_size};
            T value {};
            boost::charconv::from_chars(buf, buf + in_len[i], value, base);
            out[i] = value;
        });
    }).wait();

    int failures {0};
    for (int i {0}; i < num_elements; ++i)
    {
        if (out[i] != expected[i])
        {
            if (failures < 5)
            {
                std::cerr << "Mismatch at element " << i << " (base " << base << ")\n";
            }
            ++failures;
        }
    }

    sycl::free(in_str, q);
    sycl::free(in_len, q);
    sycl::free(out, q);
    sycl::free(expected, q);

    if (failures == 0)
    {
        std::cout << "Test PASSED\n";
        return EXIT_SUCCESS;
    }

    std::cerr << "Test FAILED with " << failures << " mismatches\n";
    return EXIT_FAILURE;
}

// num_digits: count digits on the device and compare against the host.
template <typename T>
int test_num_digits()
{
    sycl::queue q;
    std::cout << "SYCL device: " << q.get_device().get_info<sycl::info::device::name>() << "\n";

    T* in {sycl::malloc_shared<T>(num_elements, q)};
    int* out {sycl::malloc_shared<int>(num_elements, q)};

    std::mt19937_64 rng {42};
    for (int i {0}; i < num_elements; ++i)
    {
        in[i] = static_cast<T>(rng());
    }

    q.submit([&](sycl::handler& h)
    {
        h.parallel_for(sycl::range<1>(num_elements), [=](sycl::id<1> idx)
        {
            const int i {static_cast<int>(idx[0])};
            out[i] = boost::charconv::detail::num_digits(in[i]);
        });
    }).wait();

    int failures {0};
    for (int i {0}; i < num_elements; ++i)
    {
        if (out[i] != boost::charconv::detail::num_digits(in[i]))
        {
            if (failures < 5)
            {
                std::cerr << "Mismatch at element " << i << "\n";
            }
            ++failures;
        }
    }

    sycl::free(in, q);
    sycl::free(out, q);

    if (failures == 0)
    {
        std::cout << "Test PASSED\n";
        return EXIT_SUCCESS;
    }

    std::cerr << "Test FAILED with " << failures << " mismatches\n";
    return EXIT_FAILURE;
}

} // namespace charconv_sycl_test

#endif // BOOST_CHARCONV_TEST_SYCL_CHARCONV_TEST_HPP
