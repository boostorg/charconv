module;

#include <cstdint> // for UINT64_C
#include <climits> // for CHAR_BIT
#include <cfloat>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/charconv/detail/config.hpp>

export module boost.charconv;

import std;
import boost.core;

// Wrapping compiled libraries in "extern C++" breaks
// implementation units: exported declarations are no longer reachable
#include <boost/charconv.hpp>
