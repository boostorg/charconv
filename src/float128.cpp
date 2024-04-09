// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "float128_impl.hpp"

#ifdef BOOST_CHARCONV_HAS_FLOAT128

namespace boost {
namespace charconv {

namespace detail {


// --------------------------------------------------------------------------------------------------------------------
// Ryu
// --------------------------------------------------------------------------------------------------------------------

namespace ryu {

inline struct floating_decimal_128 float128_to_fd128(__float128 d) noexcept
{
#ifdef BOOST_CHARCONV_HAS_INT128
    unsigned_128_type bits = 0;
    std::memcpy(&bits, &d, sizeof(__float128));
#else
    trivial_uint128 trivial_bits;
    std::memcpy(&trivial_bits, &d, sizeof(__float128));
    unsigned_128_type bits {trivial_bits};
#endif

    return generic_binary_to_decimal(bits, 112, 15, false);
}

#ifdef BOOST_CHARCONV_HAS_STDFLOAT128

inline struct floating_decimal_128 stdfloat128_to_fd128(std::float128_t d) noexcept
{
#ifdef BOOST_CHARCONV_HAS_INT128
    unsigned_128_type bits = 0;
    std::memcpy(&bits, &d, sizeof(std::float128_t));
#else
    trivial_uint128 trivial_bits;
    std::memcpy(&trivial_bits, &d, sizeof(std::float128_t));
    unsigned_128_type bits {trivial_bits};
#endif

    return generic_binary_to_decimal(bits, 112, 15, false);
}

#endif

// --------------------------------------------------------------------------------------------------------------------
// generate nans
// --------------------------------------------------------------------------------------------------------------------

inline __float128 nans BOOST_PREVENT_MACRO_SUBSTITUTION () noexcept
{
    words bits;
    bits.hi = UINT64_C(0x7FFF400000000000);
    bits.lo = UINT64_C(0);

    __float128 return_val;
    std::memcpy(&return_val, &bits, sizeof(__float128));
    return return_val;
}

inline __float128 nanq BOOST_PREVENT_MACRO_SUBSTITUTION () noexcept
{
    words bits;
    bits.hi = UINT64_C(0x7FFF800000000000);
    bits.lo = UINT64_C(0);

    __float128 return_val;
    std::memcpy(&return_val, &bits, sizeof(__float128));
    return return_val;
}

} // namespace ryu

} // namespace detail

} // namespace charconv
} // namespace boost

#endif // BOOST_CHARCONV_HAS_FLOAT128
