module;

#include <boost/charconv/detail/global_module_fragment.hpp>

export module boost.charconv;

import std;
import boost.core;

extern "C++" {
#include <boost/charconv.hpp>
}

export namespace boost::charconv {

using charconv::chars_format;
using charconv::from_chars;
using charconv::from_chars_erange;
using charconv::limits;
using charconv::to_chars;
using charconv::from_chars_result;
using charconv::to_chars_result;

}
