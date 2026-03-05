// Module-safe wrapper for quadmath
#if defined(BOOST_IN_MODULE_PURVIEW) && !defined(BOOST_CHARCONV_QUADMATH_INCLUDED)
#  error "Please #include <quadmath.h> in your module global fragment"
#endif

#define BOOST_CHARCONV_QUADMATH_INCLUDED
#include <quadmath.h>
