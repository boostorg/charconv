// Module-safe wrapper for quadmath
#if defined(BOOST_IN_MODULE_PURVIEW) && !defined(BOOST_CHARCONV_QUADMATH_INCLUDED)
#  error "Please #include "quadmath_wrapper.hpp" in your module global fragment"
#endif

#define BOOST_CHARCONV_QUADMATH_INCLUDED

#ifdef BOOST_CHARCONV_HAS_QUADMATH
#include <quadmath.h>
#endif
