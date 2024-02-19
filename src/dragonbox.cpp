#include <boost/charconv/detail/dragonbox/dragonbox.hpp>

namespace boost { namespace charconv { namespace detail {
#if defined(BOOST_NO_CXX14_CONSTEXPR)

    constexpr int main_cache_holder::cache_bits;
    constexpr int main_cache_holder::min_k;
    constexpr int main_cache_holder::max_k;
    constexpr main_cache_holder::cache_entry_type main_cache_holder::cache[];

    constexpr int cache_holder_ieee754_binary64::cache_bits;
    constexpr int cache_holder_ieee754_binary64::min_k;
    constexpr int cache_holder_ieee754_binary64::max_k;
    constexpr cache_holder_ieee754_binary64::cache_entry_type cache_holder_ieee754_binary64::cache[];

#endif
}}}  // namespace boost::charconv::detail
