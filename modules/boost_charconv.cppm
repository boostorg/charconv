module;

#include <boost/charconv/detail/global_module_fragment.hpp>

export module boost.charconv;

import std;
import boost.core;

// Wrapping compiled libraries in "extern C++" breaks
// implementation units: exported declarations are no longer reachable
#include <boost/charconv.hpp>
