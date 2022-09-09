# mi-cpptest

This library is a intended as a minuscule unit-test library for C++11
and later:

- it should fail if a test fails
- it tries to print a message explaining the failure,
  - using `void mi_cpptest_stringify(std::ostream&, const T&)`, if defined,
  - else using `std::ostream& operator<<(std::ostream&, const T&)`, if defined,
  - else using a standard text
  to format values
- it tries to print TAP output
- it allows selecting which tests to run

The library has no dependencies beyond the standard C++ library.

## Running

At present, all command line arguments are interpreted test filters:

- `-{regex}` deselects tests matching `{regex}`
- `{regex}` selects tests matching `{regex}`
- searching the argument list stops at the first match, and the test
  is skipped if this was a regex with `-` prefix.

## Use with CMake

1. either include this as a subproject with `ADD_SUBDIRECTORY(...)`
2. or build and install and use `FIND_PACKAGE(mi-cpptest)`

In both cases, link to `mi-cpptest` or `mi-cpptest-main`.

## Use without CMake

As the library consists of a few files, it should be easy to use with
other build systems.
