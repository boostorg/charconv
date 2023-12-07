# CharConv
This library is a C++11 compatible implementation of `<charconv>`. The full documentation can be found here: https://develop.charconv.cpp.al

# Notice
This library is not an official boost library.

# How to build the library

## B2

````
git clone https://github.com/boostorg/boost
cd boost
git submodule update --init
cd libs
git clone https://github.com/cppalliance/charconv
cd ..
./bootstrap
./b2 cxxstd=11
````

This sets up a complete boost development and allows the tests to be run. To install the development environment run:

````
sudo ./b2 install cxxstd=11
````

## vcpkg

````
git clone https://github.com/cppalliance/charconv
cd charconv
vcpkg install charconv --overlay-ports=ports/charconv 
````

This will install charconv and all the required boost packages if they do not already exist.

## Conan

````
git clone https://github.com/cppalliance/charconv
conan create charconv/conan --build missing
````

This will build a boost_charconv package using your default profile and put it
in the local Conan cache along with all direct and transitive dependencies.
Since Charconv only depends on a few header-only Boost libraries, you can
save some time by requesting header-only Boost:

```
conan create charconv/conan -o 'boost*:header_only=True' --build missing
````

Following one of those approaches you can use the package as usual. For
example, using a `conanfile.txt`:

```
[requires]
boost_charconv/1.0.0
````

# Synopsis

Charconv is a collection of parsing functions that are locale-independent, non-allocating, and non-throwing.

````
namespace boost { namespace charconv {

enum class chars_format : unsigned
{
    scientific = 1 << 0,
    fixed = 1 << 1,
    hex = 1 << 2,
    general = fixed | scientific
};

struct from_chars_result
{
    const char* ptr;
    std::errc ec;

    friend constexpr bool operator==(const from_chars_result& lhs, const from_chars_result& rhs) noexcept
    friend constexpr bool operator!=(const from_chars_result& lhs, const from_chars_result& rhs) noexcept
    constexpr explicit operator bool() const noexcept
}

template <typename Integral>
BOOST_CXX14_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, Integral& value, int base = 10) noexcept;

BOOST_CXX14_CONSTEXPR from_chars_result from_chars<bool>(const char* first, const char* last, bool& value, int base) = delete;

template <typename Real>
from_chars_result from_chars(const char* first, const char* last, Real& value, chars_format fmt = chars_format::general) noexcept;

struct to_chars_result
{
    char* ptr;
    std::errc ec;

    friend constexpr bool operator==(const to_chars_result& lhs, const to_chars_result& rhs) noexcept;
    friend constexpr bool operator!=(const to_chars_result& lhs, const to_chars_result& rhs) noexcept;
    constexpr explicit operator bool() const noexcept
};

template <typename Integral>
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars(char* first, char* last, Integral value, int base = 10) noexcept;

template <typename Integral>
BOOST_CHARCONV_CONSTEXPR to_chars_result to_chars<bool>(char* first, char* last, Integral value, int base) noexcept = delete;

template <typename Real>
to_chars_result to_chars(char* first, char* last, Real value, chars_format fmt = chars_format::general, int precision) noexcept;

}} // Namespace boost::charconv
````

## Notes
- `BOOST_CXX14_CONSTEXPR` is defined as `constexpr` when compiling with C++14 or newer.

- `BOOST_CHARCONV_CONSTEXPR` is defined as `constexpr` when compiling with C++14 or newer, and the compiler has `__builtin_is_constant_evaluated`

# Examples

## `from_chars`

````
const char* buffer = "42";
int v = 0;
from_chars_result r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), v);
assert(r.ec == std::errc());
assert(v == 42);

const char* buffer = "1.2345"
double v = 0;
auto r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), v);
assert(r.ec == std::errc());
assert(v == 1.2345);

const char* buffer = "2a";
unsigned v = 0;
auto r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), v, 16);
assert(r); // from_chars_result has operator bool()
assert(v == 42);

const char* buffer = "1.3a2bp-10";
double v = 0;
auto r = boost::charconv::from_chars(buffer, buffer + std::strlen(buffer), v, boost::charconv::chars_format::hex);
assert(r);
assert(v == 8.0427e-18);
````
## `to_chars`

````
char buffer[64] {};
int v = 42;
to_chars_result r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer) - 1, v);
assert(r.ec == std::errc());
assert(!strcmp(buffer, "42")); // strcmp returns 0 on match

char buffer[64] {};
double v = 1e300;
to_chars_result r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer) - 1, v);
assert(r.ec == std::errc());
assert(!strcmp(buffer, "1e+300"));

char buffer[64] {};
int v = 42;
to_chars_result r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer) - 1, v, 16);
assert(r); // to_chars_result has operator bool()
assert(!strcmp(buffer, "2a")); // strcmp returns 0 on match

````
