// Copyright 2020-2023 Junekey Jeon
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_POLICIES_HPP
#define BOOST_CHARCONV_DETAIL_POLICIES_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/float_traits.hpp>
#include <boost/charconv/detail/dragonbox_cache.hpp>
#include <boost/charconv/detail/log.hpp>
#include <boost/charconv/detail/emulated128.hpp>
#include <utility>
#include <cstdint>
#include <cstddef>

namespace boost { namespace charconv { namespace detail {

// Forward declare the implementation class.
template <typename Float, typename FloatTraits = default_float_traits<Float>>
struct impl;

namespace policy_impl {
    // Sign policies.
    namespace sign {
        struct base {};

        struct ignore : base 
        {
            using sign_policy = ignore;
            static constexpr bool return_has_sign = false;

            template <typename SignedSignificandBits, typename ReturnType>
            BOOST_CXX14_CONSTEXPR void handle_sign(SignedSignificandBits, ReturnType&) noexcept {}
        };

        struct return_sign : base 
        {
            using sign_policy = return_sign;
            static constexpr bool return_has_sign = true;

            template <typename SignedSignificandBits, typename ReturnType>
            BOOST_CXX14_CONSTEXPR void handle_sign(SignedSignificandBits s, ReturnType& r) noexcept 
            {
                r.is_negative = s.is_negative();
            }
        };
    }

    // Trailing zero policies.
    namespace trailing_zero {
        struct base {};

        struct ignore : base 
        {
            using trailing_zero_policy = ignore;
            static constexpr bool report_trailing_zeros = false;

            template <typename Impl, typename ReturnType>
            static BOOST_CHARCONV_CXX14_CONSTEXPR void on_trailing_zeros(ReturnType&) noexcept {}

            template <typename Impl, typename ReturnType>
            static BOOST_CHARCONV_CXX14_CONSTEXPR void no_trailing_zeros(ReturnType&) noexcept {}
        };

        struct remove : base 
        {
            using trailing_zero_policy = remove;
            static constexpr bool report_trailing_zeros = false;

            template <typename Impl, typename ReturnType>
            BOOST_FORCEINLINE BOOST_CXX14_CONSTEXPR void on_trailing_zeros(ReturnType& r) noexcept 
            {
                r.exponent += Impl::remove_trailing_zeros(r.significand);
            }

            template <typename Impl, typename ReturnType>
            static BOOST_CHARCONV_CXX14_CONSTEXPR void no_trailing_zeros(ReturnType&) noexcept {}
        };

        struct report : base 
        {
            using trailing_zero_policy = report;
            static constexpr bool report_trailing_zeros = true;

            template <typename Impl, typename ReturnType>
            static BOOST_CHARCONV_CXX14_CONSTEXPR void on_trailing_zeros(ReturnType& r) noexcept
            {
                r.may_have_trailing_zeros = true;
            }

            template <typename Impl, typename ReturnType>
            static BOOST_CHARCONV_CXX14_CONSTEXPR void no_trailing_zeros(ReturnType& r) noexcept
            {
                r.may_have_trailing_zeros = false;
            }
        };
    }

    // Decimal-to-binary rounding mode policies.
    namespace decimal_to_binary_rounding {
        struct base {};

        enum class tag_t 
        {
            to_nearest,
            left_closed_directed,
            right_closed_directed
        };

        namespace interval_type {
            struct symmetric_boundary 
            {
                static constexpr bool is_symmetric = true;
                bool is_closed {};
                constexpr bool include_left_endpoint() const noexcept { return is_closed; }
                constexpr bool include_right_endpoint() const noexcept { return is_closed; }
            };

            struct asymmetric_boundary
            {
                static constexpr bool is_symmetric = false;
                bool is_left_closed {};
                constexpr bool include_left_endpoint() const noexcept { return is_left_closed; }
                constexpr bool include_right_endpoint() const noexcept { return !is_left_closed; }
            };

            struct closed 
            {
                static constexpr bool is_symmetric = true;
                static constexpr bool include_left_endpoint() noexcept { return true; }
                static constexpr bool include_right_endpoint() noexcept { return true; }
            };

            struct open 
            {
                static constexpr bool is_symmetric = true;
                static constexpr bool include_left_endpoint() noexcept { return false; }
                static constexpr bool include_right_endpoint() noexcept { return false; }
            };

            struct left_closed_right_open
            {
                static constexpr bool is_symmetric = false;
                static constexpr bool include_left_endpoint() noexcept { return true; }
                static constexpr bool include_right_endpoint() noexcept { return false; }
            };

            struct right_closed_left_open 
            {
                static constexpr bool is_symmetric = false;
                static constexpr bool include_left_endpoint() noexcept { return false; }
                static constexpr bool include_right_endpoint() noexcept { return true; }
            };
        }

        struct nearest_to_even : base 
        {
            using decimal_to_binary_rounding_policy = nearest_to_even;
            static constexpr auto tag = tag_t::to_nearest;
            using normal_interval_type = interval_type::symmetric_boundary;
            using shorter_interval_type = interval_type::closed;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto delegate(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(nearest_to_even{});
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE BOOST_CXX14_CONSTEXPR auto 
            invoke_normal_interval_case(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(s.has_even_significand_bits());
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static constexpr auto
            invoke_shorter_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f();
            }
        };

        struct nearest_to_odd : base 
        {
            using decimal_to_binary_rounding_policy = nearest_to_odd;
            static constexpr auto tag = tag_t::to_nearest;
            using normal_interval_type = interval_type::symmetric_boundary;
            using shorter_interval_type = interval_type::open;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(nearest_to_odd{});
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE BOOST_CXX14_CONSTEXPR auto
            invoke_normal_interval_case(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(!s.has_even_significand_bits());
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static constexpr auto
            invoke_shorter_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f();
            }
        };

        struct nearest_toward_plus_infinity : base
        {
            using decimal_to_binary_rounding_policy = nearest_toward_plus_infinity;
            static constexpr auto tag = tag_t::to_nearest;
            using normal_interval_type = interval_type::asymmetric_boundary;
            using shorter_interval_type = interval_type::asymmetric_boundary;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
             {
                return f(nearest_toward_plus_infinity{});
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE BOOST_CXX14_CONSTEXPR auto
            invoke_normal_interval_case(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(!s.is_negative());
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE BOOST_CXX14_CONSTEXPR auto
            invoke_shorter_interval_case(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(!s.is_negative());
            }
        };

        struct nearest_toward_minus_infinity : base
        {
            using decimal_to_binary_rounding_policy = nearest_toward_minus_infinity;
            static constexpr auto tag = tag_t::to_nearest;
            using normal_interval_type = interval_type::asymmetric_boundary;
            using shorter_interval_type = interval_type::asymmetric_boundary;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(nearest_toward_minus_infinity{});
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE BOOST_CXX14_CONSTEXPR auto
            invoke_normal_interval_case(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(s.is_negative());
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE BOOST_CXX14_CONSTEXPR auto
            invoke_shorter_interval_case(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(s.is_negative());
            }
        };

        struct nearest_toward_zero : base 
        {
            using decimal_to_binary_rounding_policy = nearest_toward_zero;
            static constexpr auto tag = tag_t::to_nearest;
            using normal_interval_type = interval_type::right_closed_left_open;
            using shorter_interval_type = interval_type::right_closed_left_open;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>()) 
            {
                return f(nearest_toward_zero{});
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static constexpr auto
            invoke_normal_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f();
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static constexpr auto
            invoke_shorter_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f();
            }
        };

        struct nearest_away_from_zero : base 
        {
            using decimal_to_binary_rounding_policy = nearest_away_from_zero;
            static constexpr auto tag = tag_t::to_nearest;
            using normal_interval_type = interval_type::left_closed_right_open;
            using shorter_interval_type = interval_type::left_closed_right_open;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(nearest_away_from_zero{});
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static constexpr auto
            invoke_normal_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f();
            }

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static constexpr auto
            invoke_shorter_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f();
            }
        };

        namespace detail {
            struct nearest_always_closed 
            {
                static constexpr auto tag = tag_t::to_nearest;
                using normal_interval_type = interval_type::closed;
                using shorter_interval_type = interval_type::closed;

                template <typename SignedSignificandBits, typename Func>
                BOOST_FORCEINLINE static constexpr auto
                invoke_normal_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
                {
                    return f();
                }
                template <typename SignedSignificandBits, typename Func>
                BOOST_FORCEINLINE static constexpr auto
                invoke_shorter_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
                {
                    return f();
                }
            };
            struct nearest_always_open {
                static constexpr auto tag = tag_t::to_nearest;
                using normal_interval_type = interval_type::open;
                using shorter_interval_type = interval_type::open;

                template <typename SignedSignificandBits, typename Func>
                BOOST_FORCEINLINE static constexpr auto
                invoke_normal_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
                {
                    return f();
                }
                template <typename SignedSignificandBits, typename Func>
                BOOST_FORCEINLINE static constexpr auto
                invoke_shorter_interval_case(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
                {
                    return f();
                }
            };
        }

        struct nearest_to_even_static_boundary : base 
        {
            using decimal_to_binary_rounding_policy = nearest_to_even_static_boundary;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                if (s.has_even_significand_bits())
                {
                    return f(detail::nearest_always_closed{});
                }
                else
                {
                    return f(detail::nearest_always_open{});
                }
            }
        };

        struct nearest_to_odd_static_boundary : base 
        {
            using decimal_to_binary_rounding_policy = nearest_to_odd_static_boundary;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                if (s.has_even_significand_bits()) 
                {
                    return f(detail::nearest_always_open{});
                }
                else 
                {
                    return f(detail::nearest_always_closed{});
                }
            }
        };

        struct nearest_toward_plus_infinity_static_boundary : base 
        {
            using decimal_to_binary_rounding_policy =
                nearest_toward_plus_infinity_static_boundary;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                if (s.is_negative()) 
                {
                    return f(nearest_toward_zero{});
                }
                else 
                {
                    return f(nearest_away_from_zero{});
                }
            }
        };

        struct nearest_toward_minus_infinity_static_boundary : base 
        {
            using decimal_to_binary_rounding_policy = nearest_toward_minus_infinity_static_boundary;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>()) 
            {
                if (s.is_negative()) 
                {
                    return f(nearest_away_from_zero{});
                }
                else 
                {
                    return f(nearest_toward_zero{});
                }
            }
        };

        namespace detail {
            struct left_closed_directed 
            {
                static constexpr auto tag = tag_t::left_closed_directed;
            };
            struct right_closed_directed 
            {
                static constexpr auto tag = tag_t::right_closed_directed;
            };
        }

        struct toward_plus_infinity : base 
        {
            using decimal_to_binary_rounding_policy = toward_plus_infinity;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>()) 
            {
                if (s.is_negative()) 
                {
                    return f(detail::left_closed_directed{});
                }
                else 
                {
                    return f(detail::right_closed_directed{});
                }
            }
        };

        struct toward_minus_infinity : base
        {
            using decimal_to_binary_rounding_policy = toward_minus_infinity;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits s, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                if (s.is_negative())
                {
                    return f(detail::right_closed_directed{});
                }
                else
                {
                    return f(detail::left_closed_directed{});
                }
            }
        };

        struct toward_zero : base 
        {
            using decimal_to_binary_rounding_policy = toward_zero;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(detail::left_closed_directed{});
            }
        };

        struct away_from_zero : base 
        {
            using decimal_to_binary_rounding_policy = away_from_zero;

            template <typename SignedSignificandBits, typename Func>
            BOOST_FORCEINLINE static auto 
            delegate(SignedSignificandBits, Func&& f) noexcept -> decltype(std::declval<Func>())
            {
                return f(detail::right_closed_directed{});
            }
        };
    }

    // Binary-to-decimal rounding policies.
    // (Always assumes nearest rounding modes.)
    namespace binary_to_decimal_rounding {
        struct base {};

        enum class tag_t 
        { 
            do_not_care, 
            to_even, to_odd, 
            away_from_zero, 
            toward_zero 
        };

        struct do_not_care : base 
        {
            using binary_to_decimal_rounding_policy = do_not_care;
            static constexpr auto tag = tag_t::do_not_care;

            template <typename ReturnType>
            static constexpr bool prefer_round_down(ReturnType const&) noexcept { return false; }
        };

        struct to_even : base 
        {
            using binary_to_decimal_rounding_policy = to_even;
            static constexpr auto tag = tag_t::to_even;

            template <typename ReturnType>
            static constexpr bool prefer_round_down(ReturnType const& r) noexcept 
            {
                return r.significand % 2 != 0;
            }
        };

        struct to_odd : base 
        {
            using binary_to_decimal_rounding_policy = to_odd;
            static constexpr auto tag = tag_t::to_odd;

            template <typename ReturnType>
            static constexpr bool prefer_round_down(ReturnType const& r) noexcept 
            {
                return r.significand % 2 == 0;
            }
        };

        struct away_from_zero : base 
        {
            using binary_to_decimal_rounding_policy = away_from_zero;
            static constexpr auto tag = tag_t::away_from_zero;

            template <typename ReturnType>
            static constexpr bool prefer_round_down(ReturnType const&) noexcept 
            {
                return false;
            }
        };

        struct toward_zero : base 
        {
            using binary_to_decimal_rounding_policy = toward_zero;
            static constexpr auto tag = tag_t::toward_zero;

            template <typename ReturnType>
            static constexpr bool prefer_round_down(ReturnType const&) noexcept 
            {
                return true;
            }
        };
    }

    // Cache policies.
    namespace cache {
        struct base {};

        struct full : base 
        {
            using cache_policy = full;

            template <typename FloatFormat>
            BOOST_CXX14_CONSTEXPR typename cache_holder<FloatFormat>::cache_entry_type get_cache(int k) noexcept 
            {
                BOOST_CHARCONV_ASSERT(k >= cache_holder<FloatFormat>::min_k &&
                        k <= cache_holder<FloatFormat>::max_k);

                return cache_holder<FloatFormat>::cache[std::size_t(
                    k - cache_holder<FloatFormat>::min_k)];
            }
        };

        struct compact : base 
        {
            using cache_policy = compact;

            template <typename FloatFormat>
            BOOST_CXX14_CONSTEXPR typename cache_holder<FloatFormat>::cache_entry_type get_cache(int k) noexcept 
            {
                BOOST_CHARCONV_ASSERT(k >= cache_holder<FloatFormat>::min_k &&
                        k <= cache_holder<FloatFormat>::max_k);

                BOOST_IF_CONSTEXPR (std::is_same<FloatFormat, ieee754_binary64>::value)
                {
                    // Compute the base index.
                    const auto cache_index =
                        static_cast<int>(static_cast<std::uint32_t>(k - cache_holder<FloatFormat>::min_k) /
                                                                    compressed_cache_detail::compression_ratio);
                    const auto kb =
                        cache_index * compressed_cache_detail::compression_ratio +
                        cache_holder<FloatFormat>::min_k;

                    const auto offset = k - kb;

                    // Get the base cache.
                    const value128 base_cache = compressed_cache_detail::cache.table[cache_index];

                    if (offset == 0) 
                    {
                        return base_cache;
                    }
                    else 
                    {
                        // Compute the required amount of bit-shift.
                        const auto alpha = floor_log2_pow10(kb + offset) - floor_log2_pow10(kb) - offset;
                        BOOST_CHARCONV_ASSERT(alpha > 0 && alpha < 64);

                        // Try to recover the real cache.
                        const auto pow5 = compressed_cache_detail::pow5.table[offset];
                        auto recovered_cache = full_multiplication(base_cache.high, pow5);
                        const auto middle_low = full_multiplication(base_cache.low, pow5);

                        recovered_cache += middle_low.high;

                        const auto high_to_middle = recovered_cache.high << (64 - alpha);
                        const auto middle_to_low = recovered_cache.low << (64 - alpha);

                        recovered_cache = value128 {
                            (recovered_cache.low >> alpha) | high_to_middle,
                            ((middle_low.low >> alpha) | middle_to_low)};

                        BOOST_CHARCONV_ASSERT(recovered_cache.low + 1 != 0);

                        recovered_cache = {recovered_cache.high, recovered_cache.low + 1};

                        return recovered_cache;
                    }
                }
                else 
                {
                    // Just use the full cache for anything other than binary64
                    return cache_holder<FloatFormat>::cache[static_cast<std::size_t>(k - cache_holder<FloatFormat>::min_k)];
                }
            }
        };
    }
}
} // Namespace detail

namespace policy {

namespace sign {
    BOOST_INLINE_VARIABLE constexpr auto ignore = detail::policy_impl::sign::ignore{};
    BOOST_INLINE_VARIABLE constexpr auto return_sign = detail::policy_impl::sign::return_sign{};
}

namespace trailing_zero {
    BOOST_INLINE_VARIABLE constexpr auto ignore = detail::policy_impl::trailing_zero::ignore{};
    BOOST_INLINE_VARIABLE constexpr auto remove = detail::policy_impl::trailing_zero::remove{};
    BOOST_INLINE_VARIABLE constexpr auto report = detail::policy_impl::trailing_zero::report{};
}

namespace decimal_to_binary_rounding {
    BOOST_INLINE_VARIABLE constexpr auto nearest_to_even =
        detail::policy_impl::decimal_to_binary_rounding::nearest_to_even{};
    BOOST_INLINE_VARIABLE constexpr auto nearest_to_odd =
        detail::policy_impl::decimal_to_binary_rounding::nearest_to_odd{};
    BOOST_INLINE_VARIABLE constexpr auto nearest_toward_plus_infinity =
        detail::policy_impl::decimal_to_binary_rounding::nearest_toward_plus_infinity{};
    BOOST_INLINE_VARIABLE constexpr auto nearest_toward_minus_infinity =
        detail::policy_impl::decimal_to_binary_rounding::nearest_toward_minus_infinity{};
    BOOST_INLINE_VARIABLE constexpr auto nearest_toward_zero =
        detail::policy_impl::decimal_to_binary_rounding::nearest_toward_zero{};
    BOOST_INLINE_VARIABLE constexpr auto nearest_away_from_zero =
        detail::policy_impl::decimal_to_binary_rounding::nearest_away_from_zero{};

    BOOST_INLINE_VARIABLE constexpr auto nearest_to_even_static_boundary =
        detail::policy_impl::decimal_to_binary_rounding::nearest_to_even_static_boundary{};
    BOOST_INLINE_VARIABLE constexpr auto nearest_to_odd_static_boundary =
        detail::policy_impl::decimal_to_binary_rounding::nearest_to_odd_static_boundary{};
    BOOST_INLINE_VARIABLE constexpr auto nearest_toward_plus_infinity_static_boundary =
        detail::policy_impl::decimal_to_binary_rounding::
            nearest_toward_plus_infinity_static_boundary{};
    BOOST_INLINE_VARIABLE constexpr auto nearest_toward_minus_infinity_static_boundary =
        detail::policy_impl::decimal_to_binary_rounding::
            nearest_toward_minus_infinity_static_boundary{};

    BOOST_INLINE_VARIABLE constexpr auto toward_plus_infinity =
        detail::policy_impl::decimal_to_binary_rounding::toward_plus_infinity{};
    BOOST_INLINE_VARIABLE constexpr auto toward_minus_infinity =
        detail::policy_impl::decimal_to_binary_rounding::toward_minus_infinity{};
    BOOST_INLINE_VARIABLE constexpr auto toward_zero =
        detail::policy_impl::decimal_to_binary_rounding::toward_zero{};
    BOOST_INLINE_VARIABLE constexpr auto away_from_zero =
        detail::policy_impl::decimal_to_binary_rounding::away_from_zero{};
}

namespace binary_to_decimal_rounding {
    BOOST_INLINE_VARIABLE constexpr auto do_not_care =
        detail::policy_impl::binary_to_decimal_rounding::do_not_care{};
    BOOST_INLINE_VARIABLE constexpr auto to_even =
        detail::policy_impl::binary_to_decimal_rounding::to_even{};
    BOOST_INLINE_VARIABLE constexpr auto to_odd =
        detail::policy_impl::binary_to_decimal_rounding::to_odd{};
    BOOST_INLINE_VARIABLE constexpr auto away_from_zero =
        detail::policy_impl::binary_to_decimal_rounding::away_from_zero{};
    BOOST_INLINE_VARIABLE constexpr auto toward_zero =
        detail::policy_impl::binary_to_decimal_rounding::toward_zero{};
}

namespace cache {
    BOOST_INLINE_VARIABLE constexpr auto full = detail::policy_impl::cache::full{};
    BOOST_INLINE_VARIABLE constexpr auto compact = detail::policy_impl::cache::compact{};
}

} // Namespace policy

////////////////////////////////////////////////////////////////////////////////////////
// Policy holder.
////////////////////////////////////////////////////////////////////////////////////////

namespace detail { namespace policy_impl {
    // The library will specify a list of accepted kinds of policies and their defaults, and
    // the user will pass a list of policies. The aim of helper classes/functions here is to
    // do the following:
    //   1. Check if the policy parameters given by the user are all valid; that means,
    //      each of them should be of the kinds specified by the library.
    //      If that's not the case, then the compilation fails.
    //   2. Check if multiple policy parameters for the same kind is specified by the user.
    //      If that's the case, then the compilation fails.
    //   3. Build a class deriving from all policies the user have given, and also from
    //      the default policies if the user did not specify one for some kinds.
    // A policy belongs to a certain kind if it is deriving from a base class.

    // For a given kind, find a policy belonging to that kind.
    // Check if there are more than one such policies.
    enum class policy_found_info 
    { 
        not_found, 
        unique, 
        repeated 
    };

    template <typename Policy, policy_found_info info>
    struct found_policy_pair 
    {
        using policy = Policy;
        static constexpr auto found_info = info;
    };

    template <typename Base, typename DefaultPolicy>
    struct base_default_pair 
    {
        using base = Base;

        template <typename FoundPolicyInfo>
        static constexpr FoundPolicyInfo get_policy_impl(FoundPolicyInfo)
        {
            return {};
        }

        template <typename FoundPolicyInfo, typename FirstPolicy, typename... RemainingPolicies>
        BOOST_CXX14_CONSTEXPR auto get_policy_impl(FoundPolicyInfo, FirstPolicy, RemainingPolicies... remainings) 
        {
            BOOST_IF_CONSTEXPR (std::is_base_of<Base, FirstPolicy>::value) 
            {
                BOOST_IF_CONSTEXPR (FoundPolicyInfo::found_info == policy_found_info::not_found) 
                {
                    return get_policy_impl(
                        found_policy_pair<FirstPolicy, policy_found_info::unique>{},
                        remainings...);
                }
                else 
                {
                    return get_policy_impl(
                        found_policy_pair<FirstPolicy, policy_found_info::repeated>{},
                        remainings...);
                }
            }
            else 
            {
                return get_policy_impl(FoundPolicyInfo{}, remainings...);
            }
        }

        template <typename... Policies>
        BOOST_CXX14_CONSTEXPR auto get_policy(Policies... policies)
        {
            return get_policy_impl(
                found_policy_pair<DefaultPolicy, policy_found_info::not_found>{},
                policies...);
        }
    };

    template <typename... BaseDefaultPairs>
    struct base_default_pair_list {};

    // Check if a given policy belongs to one of the kinds specified by the library.
    template <typename Policy>
    constexpr bool check_policy_validity(Policy, base_default_pair_list<>) 
    {
        return false;
    }

    template <typename Policy, typename FirstBaseDefaultPair, typename... RemainingBaseDefaultPairs>
    BOOST_CXX14_CONSTEXPR bool check_policy_validity(Policy, base_default_pair_list<FirstBaseDefaultPair, RemainingBaseDefaultPairs...>) 
    {
        return std::is_base_of<typename FirstBaseDefaultPair::base, Policy>::value ||
                check_policy_validity(Policy{}, base_default_pair_list<RemainingBaseDefaultPairs...>{});
    }

    template <class BaseDefaultPairList>
    constexpr bool check_policy_list_validity(BaseDefaultPairList) 
    {
        return true;
    }

    template <class BaseDefaultPairList, class FirstPolicy, class... RemainingPolicies>
    BOOST_CXX14_CONSTEXPR bool check_policy_list_validity(BaseDefaultPairList, FirstPolicy, RemainingPolicies... remaining_policies) 
    {
        return check_policy_validity(FirstPolicy{}, BaseDefaultPairList{}) &&
               check_policy_list_validity(BaseDefaultPairList{}, remaining_policies...);
    }

    // Build policy_holder.
    template <bool repeated_, typename... FoundPolicyPairs>
    struct found_policy_pair_list 
    {
        static constexpr bool repeated = repeated_;
    };

    template <typename... Policies>
    struct policy_holder : Policies... {};

    template <bool repeated, typename... FoundPolicyPairs, typename... Policies>
    BOOST_CXX14_CONSTEXPR auto make_policy_holder_impl(base_default_pair_list<>,
                                           found_policy_pair_list<repeated, FoundPolicyPairs...>,
                                           Policies...) 
    {
        return found_policy_pair_list<repeated, FoundPolicyPairs...>{};
    }

    template <typename FirstBaseDefaultPair, typename... RemainingBaseDefaultPairs, bool repeated,
              typename... FoundPolicyPairs, typename... Policies>
    BOOST_CXX14_CONSTEXPR auto make_policy_holder_impl(
        base_default_pair_list<FirstBaseDefaultPair, RemainingBaseDefaultPairs...>,
        found_policy_pair_list<repeated, FoundPolicyPairs...>, Policies... policies) 
    {
        using new_found_policy_pair = decltype(FirstBaseDefaultPair::get_policy(policies...));

        return make_policy_holder_impl(
            base_default_pair_list<RemainingBaseDefaultPairs...>{},
            found_policy_pair_list < repeated ||
                new_found_policy_pair::found_info == policy_found_info::repeated,
            new_found_policy_pair, FoundPolicyPairs... > {}, policies...);
    }

    template <bool repeated, typename... RawPolicies>
    BOOST_CXX14_CONSTEXPR auto convert_to_policy_holder(found_policy_pair_list<repeated>, RawPolicies...) 
    {
        return policy_holder<RawPolicies...>{};
    }

    template <bool repeated, typename FirstFoundPolicyPair, typename... RemainingFoundPolicyPairs,
              typename... RawPolicies>
    BOOST_CXX14_CONSTEXPR auto convert_to_policy_holder(found_policy_pair_list<repeated, FirstFoundPolicyPair,
                                                        RemainingFoundPolicyPairs...>, RawPolicies... policies) 
    {
        return convert_to_policy_holder(found_policy_pair_list<repeated, RemainingFoundPolicyPairs...>{},
                                        typename FirstFoundPolicyPair::policy{}, policies...);
    }

    template <typename BaseDefaultPairList, typename... Policies>
    constexpr auto make_policy_holder(BaseDefaultPairList, Policies... policies) 
    {
        BOOST_CHARCONV_ASSERT_MSG(check_policy_list_validity(BaseDefaultPairList{}, Policies{}...), 
                                  "An invalid policy is specified");

        using policy_pair_list = decltype(make_policy_holder_impl(
            BaseDefaultPairList{}, found_policy_pair_list<false>{}, policies...));

        BOOST_CHARCONV_ASSERT_MSG(!policy_pair_list::repeated, "Each policy should be specified at most once");

        return convert_to_policy_holder(policy_pair_list{});
    }
}} // Namespace detail::policy_impl

}} // Namespace boost::charconv

#endif // BOOST_CHARCONV_DETAIL_POLICIES_HPP
