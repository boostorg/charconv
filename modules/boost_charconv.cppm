// Copyright 2026 Ruben Perez
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Having the interface in a separate file works around this gcc bug:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124309

export module boost.charconv;
export import :interface;
