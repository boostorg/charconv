// Copyright 2020-2023 Junekey Jeon
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_DRAGONBOX_HPP
#define BOOST_CHARCONV_DETAIL_DRAGONBOX_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <boost/charconv/detail/div.hpp>
#include <boost/charconv/detail/dragonbox_cache.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <boost/charconv/detail/float_traits.hpp>
#include <boost/charconv/detail/log.hpp>
#include <boost/charconv/detail/policies.hpp>
#include <boost/charconv/detail/power_factor.hpp>
#include <boost/core/bit.hpp>
#include <type_traits>
#include <cstdint>

namespace boost { namespace charconv { namespace detail {

////////////////////////////////////////////////////////////////////////////////////////
// The main algorithm.
////////////////////////////////////////////////////////////////////////////////////////

template <typename Float, typename FloatTraits>
struct impl : private FloatTraits, private FloatTraits::format 
{
    using format = typename FloatTraits::format;
    using carrier_uint = typename FloatTraits::carrier_uint;

    using FloatTraits::carrier_bits;
    using format::significand_bits;
    using format::min_exponent;
    using format::max_exponent;
    using format::exponent_bias;
    using format::decimal_digits;

    static constexpr int kappa = std::is_same<format, ieee754_binary32>::value ? 1 : 2;
    static_assert(kappa >= 1, "Kappa must be greater than or equal to 1");

    static BOOST_CXX14_CONSTEXPR const int min_k = [] {
        BOOST_CXX14_CONSTEXPR auto a = -floor_log10_pow2_minus_log10_4_over_3(
            static_cast<int>(max_exponent - significand_bits));
        BOOST_CXX14_CONSTEXPR auto b =
            -floor_log10_pow2(int(max_exponent - significand_bits)) + kappa;
        return a < b ? a : b;
    }();

    static BOOST_CXX14_CONSTEXPR const int max_k = [] {
        // We do invoke shorter_interval_case for exponent == min_exponent case,
        // so we should not add 1 here.
        BOOST_CXX14_CONSTEXPR auto a = -floor_log10_pow2_minus_log10_4_over_3(
            static_cast<int>(min_exponent - significand_bits /*+ 1*/));
        BOOST_CXX14_CONSTEXPR auto b =
            -floor_log10_pow2(static_cast<int>(min_exponent - significand_bits)) + kappa;
        return a > b ? a : b;
    }();

    using cache_entry_type = typename cache_holder<format>::cache_entry_type;
    static constexpr auto cache_bits = cache_holder<format>::cache_bits;

    static constexpr int case_shorter_interval_left_endpoint_lower_threshold = 2;
    static BOOST_CXX14_CONSTEXPR const int case_shorter_interval_left_endpoint_upper_threshold =
        2 + floor_log2(compute_power<count_factors<5>((carrier_uint(1) << (significand_bits + 2)) - 1) + 1>(10) / 3);

    static constexpr int case_shorter_interval_right_endpoint_lower_threshold = 0;
    static BOOST_CXX14_CONSTEXPR const int case_shorter_interval_right_endpoint_upper_threshold =
        2 + floor_log2(compute_power<count_factors<5>((carrier_uint(1) << (significand_bits + 1)) + 1) + 1>(10) / 3);

    static constexpr int shorter_interval_tie_lower_threshold =
        -floor_log5_pow2_minus_log5_3(significand_bits + 4) - 2 - significand_bits;
    static constexpr int shorter_interval_tie_upper_threshold =
        -floor_log5_pow2(significand_bits + 2) - 2 - significand_bits;

    struct compute_mul_result 
    {
        carrier_uint result;
        bool is_integer;
    };

    struct compute_mul_parity_result
    {
        bool parity;
        bool is_integer;
    };

    //// The main algorithm assumes the input is a normal/subnormal finite number

    template <typename ReturnType, typename IntervalType, typename TrailingZeroPolicy,
              typename BinaryToDecimalRoundingPolicy, typename CachePolicy, typename... AdditionalArgs>
    BOOST_CHARCONV_SAFEBUFFERS static ReturnType compute_nearest_normal(carrier_uint two_fc, const int exponent,
                                                                        AdditionalArgs... additional_args) noexcept 
    {
        //////////////////////////////////////////////////////////////////////
        // Step 1: Schubfach multiplier calculation
        //////////////////////////////////////////////////////////////////////

        ReturnType ret_value;
        IntervalType interval_type{additional_args...};

        // Compute k and beta.
        const int minus_k = floor_log10_pow2(exponent) - kappa;
        const auto cache = CachePolicy::template get_cache<format>(-minus_k);
        const int beta = exponent + floor_log2_pow10(-minus_k);

        // Compute zi and deltai.
        // 10^kappa <= deltai < 10^(kappa + 1)
        const auto deltai = compute_delta(cache, beta);
        // For the case of binary32, the result of integer check is not correct for
        // 29711844 * 2^-82
        // = 6.1442653300000000008655037797566933477355632930994033813476... * 10^-18
        // and 29711844 * 2^-81
        // = 1.2288530660000000001731007559513386695471126586198806762695... * 10^-17,
        // and they are the unique counterexamples. However, since 29711844 is even,
        // this does not cause any problem for the endpoints calculations; it can only
        // cause a problem when we need to perform integer check for the center.
        // Fortunately, with these inputs, that branch is never executed, so we are fine.
        //const auto [zi, is_z_integer] = compute_mul((two_fc | 1) << beta, cache);
        compute_mul_result z_mull = compute_mul((two_fc | 1) << beta, cache);
        const auto zi = z_mull.result;
        const auto is_z_integer = z_mull.is_integer;

        //////////////////////////////////////////////////////////////////////
        // Step 2: Try larger divisor; remove trailing zeros if necessary
        //////////////////////////////////////////////////////////////////////

        BOOST_CXX14_CONSTEXPR auto big_divisor = compute_power<kappa + 1>(UINT32_C(10));
        BOOST_CXX14_CONSTEXPR auto small_divisor = compute_power<kappa>(UINT32_C(10));

        // Using an upper bound on zi, we might be able to optimize the division
        // better than the compiler; we are computing zi / big_divisor here.
        ret_value.significand =
            divide_by_pow10<kappa + 1, carrier_uint,
                                    (carrier_uint(1) << (significand_bits + 1)) * big_divisor -
                                        1>(zi);
        auto r = static_cast<std::uint32_t>(zi - big_divisor * ret_value.significand);

        if (r < deltai) 
        {
            // Exclude the right endpoint if necessary.
            if (r == 0 && (is_z_integer & !interval_type.include_right_endpoint())) 
            {
                BOOST_IF_CONSTEXPR (BinaryToDecimalRoundingPolicy::tag == policy_impl::binary_to_decimal_rounding::tag_t::do_not_care) 
                {
                    ret_value.significand *= 10;
                    ret_value.exponent = minus_k + kappa;
                    --ret_value.significand;
                    TrailingZeroPolicy::template no_trailing_zeros<impl>(ret_value);
                    return ret_value;
                }
                else 
                {
                    --ret_value.significand;
                    r = big_divisor;
                    goto small_divisor_case_label;
                }
            }
        }
        else if (r > deltai) 
        {
            goto small_divisor_case_label;
        }
        else 
        {
            // r == deltai; compare fractional parts.
            //const auto [xi_parity, x_is_integer] = compute_mul_parity(two_fc - 1, cache, beta);
            const compute_mul_parity_result xi_result = compute_mul_parity(two_fc - 1, cache, beta);
            const auto xi_parity = xi_result.parity;
            const auto x_is_integer = xi_result.is_integer;

            if (!(xi_parity | (x_is_integer & interval_type.include_left_endpoint()))) 
            {
                goto small_divisor_case_label;
            }
        }

        ret_value.exponent = minus_k + kappa + 1;

        // We may need to remove trailing zeros.
        TrailingZeroPolicy::template on_trailing_zeros<impl>(ret_value);
        return ret_value;


        //////////////////////////////////////////////////////////////////////
        // Step 3: Find the significand with the smaller divisor
        //////////////////////////////////////////////////////////////////////

    small_divisor_case_label:
        TrailingZeroPolicy::template no_trailing_zeros<impl>(ret_value);
        ret_value.significand *= 10;
        ret_value.exponent = minus_k + kappa;

        BOOST_IF_CONSTEXPR (BinaryToDecimalRoundingPolicy::tag == policy_impl::binary_to_decimal_rounding::tag_t::do_not_care)
        {
            // Normally, we want to compute
            // ret_value.significand += r / small_divisor
            // and return, but we need to take care of the case that the resulting
            // value is exactly the right endpoint, while that is not included in the
            // interval.
            if (!interval_type.include_right_endpoint()) 
            {
                // Is r divisible by 10^kappa?
                if (is_z_integer && check_divisibility_and_divide_by_pow10<kappa>(r)) 
                {
                    // This should be in the interval.
                    ret_value.significand += r - 1;
                }
                else 
                {
                    ret_value.significand += r;
                }
            }
            else 
            {
                ret_value.significand += small_division_by_pow10<kappa>(r);
            }
        }
        else 
        {
            auto dist = r - (deltai / 2) + (small_divisor / 2);
            const bool approx_y_parity = ((dist ^ (small_divisor / 2)) & 1) != 0;

            // Is dist divisible by 10^kappa?
            const bool divisible_by_small_divisor = check_divisibility_and_divide_by_pow10<kappa>(dist);

            // Add dist / 10^kappa to the significand.
            ret_value.significand += dist;

            if (divisible_by_small_divisor)
            {
                // Check z^(f) >= epsilon^(f).
                // We have either yi == zi - epsiloni or yi == (zi - epsiloni) - 1,
                // where yi == zi - epsiloni if and only if z^(f) >= epsilon^(f).
                // Since there are only 2 possibilities, we only need to care about the
                // parity. Also, zi and r should have the same parity since the divisor is
                // an even number.
                // const auto [yi_parity, is_y_integer] = compute_mul_parity(two_fc, cache, beta);
                const compute_mul_parity_result yi_result = compute_mul_parity(two_fc, cache, beta);
                const auto yi_parity = yi_result.parity;
                const auto is_y_integer = yi_result.is_integer;

                if (yi_parity != approx_y_parity) 
                {
                    --ret_value.significand;
                }
                else 
                {
                    // If z^(f) >= epsilon^(f), we might have a tie
                    // when z^(f) == epsilon^(f), or equivalently, when y is an integer.
                    // For tie-to-up case, we can just choose the upper one.
                    if (BinaryToDecimalRoundingPolicy::prefer_round_down(ret_value) & is_y_integer) 
                    {
                        --ret_value.significand;
                    }
                }
            }
        }
        return ret_value;
    }

    template <typename ReturnType, typename IntervalType, typename TrailingZeroPolicy,
              typename BinaryToDecimalRoundingPolicy, typename CachePolicy, typename... AdditionalArgs>
    BOOST_CHARCONV_SAFEBUFFERS static ReturnType compute_nearest_shorter(const int exponent, AdditionalArgs... additional_args) noexcept 
    {
        ReturnType ret_value;
        IntervalType interval_type{additional_args...};

        // Compute k and beta.
        const int minus_k = floor_log10_pow2_minus_log10_4_over_3(exponent);
        const int beta = exponent + floor_log2_pow10(-minus_k);

        // Compute xi and zi.
        const auto cache = CachePolicy::template get_cache<format>(-minus_k);

        auto xi = compute_left_endpoint_for_shorter_interval_case(cache, beta);
        auto zi = compute_right_endpoint_for_shorter_interval_case(cache, beta);

        // If we don't accept the right endpoint and
        // if the right endpoint is an integer, decrease it.
        if (!interval_type.include_right_endpoint() && is_right_endpoint_integer_shorter_interval(exponent))
        {
            --zi;
        }
        // If we don't accept the left endpoint or
        // if the left endpoint is not an integer, increase it.
        if (!interval_type.include_left_endpoint() || !is_left_endpoint_integer_shorter_interval(exponent))
        {
            ++xi;
        }

        // Try bigger divisor.
        ret_value.significand = zi / 10;

        // If succeed, remove trailing zeros if necessary and return.
        if (ret_value.significand * 10 >= xi) 
        {
            ret_value.exponent = minus_k + 1;
            TrailingZeroPolicy::template on_trailing_zeros<impl>(ret_value);
            return ret_value;
        }

        // Otherwise, compute the round-up of y.
        TrailingZeroPolicy::template no_trailing_zeros<impl>(ret_value);
        ret_value.significand = compute_round_up_for_shorter_interval_case(cache, beta);
        ret_value.exponent = minus_k;

        // When tie occurs, choose one of them according to the rule.
        if (BinaryToDecimalRoundingPolicy::prefer_round_down(ret_value) &&
            exponent >= shorter_interval_tie_lower_threshold &&
            exponent <= shorter_interval_tie_upper_threshold) 
        {
            --ret_value.significand;
        }
        else if (ret_value.significand < xi) 
        {
            ++ret_value.significand;
        }

        return ret_value;
    }

    template <typename ReturnType, typename TrailingZeroPolicy, typename CachePolicy>
    BOOST_CHARCONV_SAFEBUFFERS static ReturnType compute_left_closed_directed(carrier_uint two_fc, int exponent) noexcept 
    {
        //////////////////////////////////////////////////////////////////////
        // Step 1: Schubfach multiplier calculation
        //////////////////////////////////////////////////////////////////////

        ReturnType ret_value;

        // Compute k and beta.
        const int minus_k = floor_log10_pow2(exponent) - kappa;
        const auto cache = CachePolicy::template get_cache<format>(-minus_k);
        const int beta = exponent + floor_log2_pow10(-minus_k);

        // Compute xi and deltai.
        // 10^kappa <= deltai < 10^(kappa + 1)
        const auto deltai = compute_delta(cache, beta);
        const auto x_result = compute_mul(two_fc << beta, cache);
        auto xi = x_result.result;
        auto is_x_integer = x_result.is_integer;

        // Deal with the unique exceptional cases
        // 29711844 * 2^-82
        // = 6.1442653300000000008655037797566933477355632930994033813476... * 10^-18
        // and 29711844 * 2^-81
        // = 1.2288530660000000001731007559513386695471126586198806762695... * 10^-17
        // for binary32.
        BOOST_IF_CONSTEXPR (std::is_same<format, ieee754_binary32>::value)
        {
            if (exponent <= -80)
            {
                is_x_integer = false;
            }
        }

        if (!is_x_integer) 
        {
            ++xi;
        }

        //////////////////////////////////////////////////////////////////////
        // Step 2: Try larger divisor; remove trailing zeros if necessary
        //////////////////////////////////////////////////////////////////////

        BOOST_CXX14_CONSTEXPR auto big_divisor = compute_power<kappa + 1>(UINT32_C(10));

        // Using an upper bound on xi, we might be able to optimize the division
        // better than the compiler; we are computing xi / big_divisor here.
        ret_value.significand =
            divide_by_pow10<kappa + 1, carrier_uint, (static_cast<carrier_uint>(1) << (significand_bits + 1)) * big_divisor - 1>(xi);

        auto r = static_cast<std::uint32_t>(xi - big_divisor * ret_value.significand);

        if (r != 0) 
        {
            ++ret_value.significand;
            r = big_divisor - r;
        }

        if (r > deltai) 
        {
            goto small_divisor_case_label;
        }
        else if (r == deltai) 
        {
            // Compare the fractional parts.
            // This branch is never taken for the exceptional cases
            // 2f_c = 29711482, e = -81
            // (6.1442649164096937243516663440523473127541365101933479309082... * 10^-18)
            // and 2f_c = 29711482, e = -80
            // (1.2288529832819387448703332688104694625508273020386695861816... * 10^-17).
            //const auto [zi_parity, is_z_integer] = compute_mul_parity(two_fc + 2, cache, beta);
            const compute_mul_parity_result z_result = compute_mul_parity(two_fc + 2, cache, beta);
            const auto zi_parity = z_result.parity;
            const auto is_z_integer = z_result.is_integer;
            
            if (zi_parity || is_z_integer) 
            {
                goto small_divisor_case_label;
            }
        }

        // The ceiling is inside, so we are done.
        ret_value.exponent = minus_k + kappa + 1;
        TrailingZeroPolicy::template on_trailing_zeros<impl>(ret_value);
        return ret_value;


        //////////////////////////////////////////////////////////////////////
        // Step 3: Find the significand with the smaller divisor
        //////////////////////////////////////////////////////////////////////

    small_divisor_case_label:
        ret_value.significand *= 10;
        ret_value.significand -= small_division_by_pow10<kappa>(r);
        ret_value.exponent = minus_k + kappa;
        TrailingZeroPolicy::template no_trailing_zeros<impl>(ret_value);
        return ret_value;
    }

    template <typename ReturnType, typename TrailingZeroPolicy, typename CachePolicy>
    BOOST_CHARCONV_SAFEBUFFERS static ReturnType compute_right_closed_directed(carrier_uint two_fc, const int exponent,
                                                                               bool shorter_interval) noexcept 
    {
        //////////////////////////////////////////////////////////////////////
        // Step 1: Schubfach multiplier calculation
        //////////////////////////////////////////////////////////////////////

        ReturnType ret_value;

        // Compute k and beta.
        const int minus_k = floor_log10_pow2(exponent - (shorter_interval ? 1 : 0)) - kappa;
        const auto cache = CachePolicy::template get_cache<format>(-minus_k);
        const int beta = exponent + floor_log2_pow10(-minus_k);

        // Compute zi and deltai.
        // 10^kappa <= deltai < 10^(kappa + 1)
        const auto deltai = shorter_interval ? compute_delta(cache, beta - 1) : compute_delta(cache, beta);
        carrier_uint zi = compute_mul(two_fc << beta, cache).result;


        //////////////////////////////////////////////////////////////////////
        // Step 2: Try larger divisor; remove trailing zeros if necessary
        //////////////////////////////////////////////////////////////////////

        constexpr auto big_divisor = compute_power<kappa + 1>(UINT32_C(10));

        // Using an upper bound on zi, we might be able to optimize the division better than
        // the compiler; we are computing zi / big_divisor here.
        ret_value.significand =
            divide_by_pow10<kappa + 1, carrier_uint, (carrier_uint(1) << (significand_bits + 1)) * big_divisor - 1>(zi);
        const auto r = static_cast<std::uint32_t>(zi - big_divisor * ret_value.significand);

        if (r > deltai) 
        {
            goto small_divisor_case_label;
        }
        else if (r == deltai) 
        {
            // Compare the fractional parts.
            if (!compute_mul_parity(two_fc - (shorter_interval ? 1 : 2), cache, beta).parity) 
            {
                goto small_divisor_case_label;
            }
        }

        // The floor is inside, so we are done.
        ret_value.exponent = minus_k + kappa + 1;
        TrailingZeroPolicy::template on_trailing_zeros<impl>(ret_value);
        return ret_value;


        //////////////////////////////////////////////////////////////////////
        // Step 3: Find the significand with the small divisor
        //////////////////////////////////////////////////////////////////////

    small_divisor_case_label:
        ret_value.significand *= 10;
        ret_value.significand += small_division_by_pow10<kappa>(r);
        ret_value.exponent = minus_k + kappa;
        TrailingZeroPolicy::template no_trailing_zeros<impl>(ret_value);
        return ret_value;
    }

    // Remove trailing zeros from n and return the number of zeros removed.
    BOOST_FORCEINLINE static int remove_trailing_zeros(carrier_uint& n) noexcept 
    {
        BOOST_CHARCONV_ASSERT(n != 0);

        BOOST_IF_CONSTEXPR (std::is_same<format, ieee754_binary32>::value) 
        {
            constexpr auto mod_inv_5 = UINT32_C(0xcccccccd);
            constexpr auto mod_inv_25 = mod_inv_5 * mod_inv_5;

            int s = 0;
            while (true) 
            {
                auto q = boost::core::rotr(n * mod_inv_25, 2);
                if (q <= (std::numeric_limits<std::uint32_t>::max)() / 100) 
                {
                    n = q;
                    s += 2;
                }
                else 
                {
                    break;
                }
            }
            auto q = boost::core::rotr(n * mod_inv_5, 1);
            if (q <= (std::numeric_limits<std::uint32_t>::max)() / 10)
            {
                n = q;
                s |= 1;
            }

            return s;
        }
        else 
        {
            static_assert(std::is_same<format, ieee754_binary64>::value, "Type must be binary64");

            // Divide by 10^8 and reduce to 32-bits if divisible.
            // Since ret_value.significand <= (2^53 * 1000 - 1) / 1000 < 10^16,
            // n is at most of 16 digits.

            // This magic number is ceil(2^90 / 10^8).
            constexpr auto magic_number = UINT64_C(12379400392853802749);
            auto nm = full_multiplication(n, magic_number);

            // Is n is divisible by 10^8?
            if ((nm.high & ((UINT64_C(1) << (90 - 64)) - 1)) == 0 && nm.low < magic_number)
            {
                // If yes, work with the quotient.
                auto n32 = static_cast<std::uint32_t>(nm.high >> (90 - 64));

                constexpr auto mod_inv_5 = UINT32_C(0xcccccccd);
                constexpr auto mod_inv_25 = mod_inv_5 * mod_inv_5;

                int s = 8;
                while (true) 
                {
                    auto q = boost::core::rotr(n32 * mod_inv_25, 2);
                    if (q <= (std::numeric_limits<std::uint32_t>::max)() / 100) 
                    {
                        n32 = q;
                        s += 2;
                    }
                    else 
                    {
                        break;
                    }
                }

                auto q = boost::core::rotr(n32 * mod_inv_5, 1);
                if (q <= (std::numeric_limits<std::uint32_t>::max)() / 10) 
                {
                    n32 = q;
                    s |= 1;
                }

                n = n32;
                return s;
            }

            // If n is not divisible by 10^8, work with n itself.
            constexpr auto mod_inv_5 = UINT64_C(0xcccccccccccccccd);
            constexpr auto mod_inv_25 = mod_inv_5 * mod_inv_5;

            int s = 0;
            while (true) 
            {
                auto q = boost::core::rotr(n * mod_inv_25, 2);
                if (q <= (std::numeric_limits<std::uint64_t>::max)() / 100)
                {
                    n = q;
                    s += 2;
                }
                else {
                    break;
                }
            }

            auto q = boost::core::rotr(n * mod_inv_5, 1);
            if (q <= (std::numeric_limits<std::uint64_t>::max)() / 10) 
            {
                n = q;
                s |= 1;
            }

            return s;
        }
    }

    static compute_mul_result compute_mul(carrier_uint u, cache_entry_type const& cache) noexcept 
    {
        BOOST_IF_CONSTEXPR (std::is_same<format, ieee754_binary32>::value) 
        {
            auto r = umul96_upper64(u, cache);
            return {carrier_uint(r >> 32), carrier_uint(r) == 0};
        }
        else 
        {
            static_assert(std::is_same<format, ieee754_binary64>::value, "Type must be binary64");
            auto r = umul192_upper128(u, cache);
            return {r.high, r.low == 0};
        }
    }

    static BOOST_CHARCONV_CXX14_CONSTEXPR std::uint32_t compute_delta(cache_entry_type const& cache, int beta) noexcept 
    {
        BOOST_IF_CONSTEXPR (std::is_same<format, ieee754_binary32>::value) 
        {
            return static_cast<std::uint32_t>(cache >> (cache_bits - 1 - beta));
        }
        else 
        {
            static_assert(std::is_same<format, ieee754_binary64>::value, "Type must be binary64");
            return static_cast<std::uint32_t>(cache.high >> (carrier_bits - 1 - beta));
        }
    }

    static compute_mul_parity_result compute_mul_parity(carrier_uint two_f, cache_entry_type const& cache, int beta) noexcept 
    {
        BOOST_CHARCONV_ASSERT(beta >= 1 && beta < 64);

        BOOST_IF_CONSTEXPR (std::is_same<format, ieee754_binary32>::value) 
        {
            auto r = umul96_lower64(two_f, cache);
            return {((r >> (64 - beta)) & 1) != 0, std::uint32_t(r >> (32 - beta)) == 0};
        }
        else 
        {
            static_assert(std::is_same<format, ieee754_binary64>::value, "Type must be binary64");
            auto r = umul192_lower128(two_f, cache);
            return {((r.high >> (64 - beta)) & 1) != 0, ((r.high << beta) | (r.low >> (64 - beta))) == 0};
        }
    }

    static BOOST_CHARCONV_CXX14_CONSTEXPR carrier_uint compute_left_endpoint_for_shorter_interval_case(cache_entry_type const& cache,
                                                                                                       int beta) noexcept 
    {
        BOOST_IF_CONSTEXPR (std::is_same<format, ieee754_binary32>::value)
        {
            return static_cast<carrier_uint>((cache - (cache >> (significand_bits + 2))) >> (cache_bits - significand_bits - 1 - beta));
        }
        else 
        {
            static_assert(std::is_same<format, ieee754_binary64>::value, "Type must be binary64");
            return (cache.high - (cache.high >> (significand_bits + 2))) >> (carrier_bits - significand_bits - 1 - beta);
        }
    }

    static BOOST_CHARCONV_CXX14_CONSTEXPR carrier_uint compute_right_endpoint_for_shorter_interval_case(cache_entry_type const& cache,
                                                                                                        int beta) noexcept 
    {
        BOOST_IF_CONSTEXPR (std::is_same<format, ieee754_binary32>::value) 
        {
            return static_cast<carrier_uint>((cache + (cache >> (significand_bits + 1))) >> (cache_bits - significand_bits - 1 - beta));
        }
        else
        {
            static_assert(std::is_same<format, ieee754_binary64>::value, "Type must be binary64");
            return (cache.high + (cache.high >> (significand_bits + 1))) >> (carrier_bits - significand_bits - 1 - beta);
        }
    }

    static BOOST_CHARCONV_CXX14_CONSTEXPR carrier_uint compute_round_up_for_shorter_interval_case(cache_entry_type const& cache,
                                                                                                  int beta) noexcept {
        BOOST_IF_CONSTEXPR (std::is_same<format, ieee754_binary32>::value) 
        {
            return (carrier_uint(cache >> (cache_bits - significand_bits - 2 - beta)) + 1) / 2;
        }
        else 
        {
            static_assert(std::is_same<format, ieee754_binary64>::value, "Type must be binary64");
            return ((cache.high >> (carrier_bits - significand_bits - 2 - beta)) + 1) / 2;
        }
    }

    static constexpr bool is_right_endpoint_integer_shorter_interval(int exponent) noexcept 
    {
        return exponent >= case_shorter_interval_right_endpoint_lower_threshold &&
               exponent <= case_shorter_interval_right_endpoint_upper_threshold;
    }

    static constexpr bool is_left_endpoint_integer_shorter_interval(int exponent) noexcept 
    {
        return exponent >= case_shorter_interval_left_endpoint_lower_threshold &&
               exponent <= case_shorter_interval_left_endpoint_upper_threshold;
    }
};

////////////////////////////////////////////////////////////////////////////////////////
// The interface function.
////////////////////////////////////////////////////////////////////////////////////////

template <typename Float, typename FloatTraits = default_float_traits<Float>, typename... Policies>
BOOST_FORCEINLINE BOOST_CHARCONV_SAFEBUFFERS auto to_decimal(signed_significand_bits<Float, FloatTraits> signed_significand_bits,
                                                             unsigned int exponent_bits, Policies... policies) noexcept 
{
    // Build policy holder type.
    using namespace detail::policy_impl;
    using policy_holder = decltype(make_policy_holder(
        base_default_pair_list<base_default_pair<sign::base, sign::return_sign>,
                                base_default_pair<trailing_zero::base, trailing_zero::remove>,
                                base_default_pair<decimal_to_binary_rounding::base,
                                                    decimal_to_binary_rounding::nearest_to_even>,
                                base_default_pair<binary_to_decimal_rounding::base,
                                                    binary_to_decimal_rounding::to_even>,
                                base_default_pair<cache::base, cache::full>>{}, policies...));

    using return_type = decimal_fp<typename FloatTraits::carrier_uint, policy_holder::return_has_sign, 
                                   policy_holder::report_trailing_zeros>;

    return_type ret = policy_holder::delegate(
        signed_significand_bits,
        [exponent_bits, signed_significand_bits](auto interval_type_provider) {
            using format = typename FloatTraits::format;
            constexpr auto tag = decltype(interval_type_provider)::tag;

            auto two_fc = signed_significand_bits.remove_sign_bit_and_shift();
            auto exponent = int(exponent_bits);

            BOOST_IF_CONSTEXPR (tag == decimal_to_binary_rounding::tag_t::to_nearest)
            {
                // Is the input a normal number?
                if (exponent != 0)
                {
                    exponent += format::exponent_bias - format::significand_bits;

                    // Shorter interval case; proceed like Schubfach.
                    // One might think this condition is wrong, since when exponent_bits == 1
                    // and two_fc == 0, the interval is actually regular. However, it turns out
                    // that this seemingly wrong condition is actually fine, because the end
                    // result is anyway the same.
                    //
                    // [binary32]
                    // (fc-1/2) * 2^e = 1.175'494'28... * 10^-38
                    // (fc-1/4) * 2^e = 1.175'494'31... * 10^-38
                    //    fc    * 2^e = 1.175'494'35... * 10^-38
                    // (fc+1/2) * 2^e = 1.175'494'42... * 10^-38
                    //
                    // Hence, shorter_interval_case will return 1.175'494'4 * 10^-38.
                    // 1.175'494'3 * 10^-38 is also a correct shortest representation that will
                    // be rejected if we assume shorter interval, but 1.175'494'4 * 10^-38 is
                    // closer to the true value so it doesn't matter.
                    //
                    // [binary64]
                    // (fc-1/2) * 2^e = 2.225'073'858'507'201'13... * 10^-308
                    // (fc-1/4) * 2^e = 2.225'073'858'507'201'25... * 10^-308
                    //    fc    * 2^e = 2.225'073'858'507'201'38... * 10^-308
                    // (fc+1/2) * 2^e = 2.225'073'858'507'201'63... * 10^-308
                    //
                    // Hence, shorter_interval_case will return 2.225'073'858'507'201'4 *
                    // 10^-308. This is indeed of the shortest length, and it is the unique one
                    // closest to the true value among valid representations of the same length.
                    static_assert(std::is_same<format, ieee754_binary32>::value ||
                                  std::is_same<format, ieee754_binary64>::value, "Type must be either binary 32 or 64");

                    if (two_fc == 0) 
                    {
                        return decltype(interval_type_provider)::invoke_shorter_interval_case(
                            signed_significand_bits, [exponent](auto... additional_args) {
                                return detail::impl<Float, FloatTraits>::
                                    template compute_nearest_shorter<
                                        return_type,
                                        typename decltype(interval_type_provider)::
                                            shorter_interval_type,
                                        typename policy_holder::trailing_zero_policy,
                                        typename policy_holder::
                                            binary_to_decimal_rounding_policy,
                                        typename policy_holder::cache_policy>(
                                        exponent, additional_args...);
                            });
                    }

                    two_fc |= (static_cast<decltype(two_fc)>(1) << (format::significand_bits + 1));
                }
                // Is the input a subnormal number?
                else 
                {
                    exponent = format::min_exponent - format::significand_bits;
                }

                return decltype(interval_type_provider)::invoke_normal_interval_case(
                    signed_significand_bits, [two_fc, exponent](auto... additional_args) {
                        return detail::impl<Float, FloatTraits>::
                            template compute_nearest_normal<
                                return_type,
                                typename decltype(interval_type_provider)::normal_interval_type,
                                typename policy_holder::trailing_zero_policy,
                                typename policy_holder::binary_to_decimal_rounding_policy,
                                typename policy_holder::cache_policy>(two_fc, exponent,
                                                                        additional_args...);
                    });
            }
            else BOOST_IF_CONSTEXPR (tag == decimal_to_binary_rounding::tag_t::left_closed_directed)
            {
                // Is the input a normal number?
                if (exponent != 0) 
                {
                    exponent += format::exponent_bias - format::significand_bits;
                    two_fc |= (static_cast<decltype(two_fc)>(1) << (format::significand_bits + 1));
                }
                // Is the input a subnormal number?
                else 
                {
                    exponent = format::min_exponent - format::significand_bits;
                }

                return detail::impl<Float>::template compute_left_closed_directed<
                    return_type, typename policy_holder::trailing_zero_policy,
                    typename policy_holder::cache_policy>(two_fc, exponent);
            }
            else 
            {
                static_assert(tag == decimal_to_binary_rounding::tag_t::right_closed_directed, "Tag must be right_closed_direction");

                bool shorter_interval {};

                // Is the input a normal number?
                if (exponent != 0) 
                {
                    if (two_fc == 0 && exponent != 1) 
                    {
                        shorter_interval = true;
                    }

                    exponent += format::exponent_bias - format::significand_bits;
                    two_fc |= (static_cast<decltype(two_fc)>(1) << (format::significand_bits + 1));
                }
                // Is the input a subnormal number?
                else 
                {
                    exponent = format::min_exponent - format::significand_bits;
                }

                return detail::impl<Float>::template compute_right_closed_directed<
                    return_type, typename policy_holder::trailing_zero_policy,
                    typename policy_holder::cache_policy>(two_fc, exponent, shorter_interval);
            }
        });

    policy_holder::handle_sign(signed_significand_bits, ret);
    return ret;
}

template <typename Float, typename FloatTraits = default_float_traits<Float>, typename... Policies>
BOOST_FORCEINLINE BOOST_CHARCONV_SAFEBUFFERS auto to_decimal(Float x, Policies... policies) noexcept 
{
    const auto br = float_bits<Float, FloatTraits>(x);
    const auto exponent_bits = br.extract_exponent_bits();
    const auto s = br.remove_exponent_bits(exponent_bits);
    BOOST_CHARCONV_ASSERT(br.is_finite());

    return to_decimal<Float, FloatTraits>(s, exponent_bits, policies...);
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_DRAGONBOX_HPP
