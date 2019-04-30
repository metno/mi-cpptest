/* -*- c++ -*-
  mi-cpptest

  Copyright (C) 2019 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no

  This file is part of mi-cpptest.

  mi-cpptest is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  mi-cpptest is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with mi-cpptest; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MI_CPPTEST_H
#define MI_CPPTEST_H

#include <cmath>
#include <cstdlib>
#include <memory>

namespace miutil {
namespace cpptest {

typedef void (*test_function_t)();

class test_failure : public std::exception {
public:
    test_failure(const char* file, int lineno)
        : file_(file), lineno_(lineno) { }

    const char* file() const { return file_; }
    int lineno() const { return lineno_; }

private:
    const char* file_;
    int lineno_;
};

bool register_test(const char* name, test_function_t tf);

bool run_tests(size_t npatterns, char* patterns[]);

bool check_close(double a, double b, double tol);

void ensure(bool b, const char* file, int lineno);

} // namespace cpptest
} // namespace miutil

#define MI_CPPTEST_TEST_CASE(x)                                         \
    static void x();                                                    \
    static bool test4fimex_registered_##x = miutil::cpptest::register_test(#x, x); \
    static void x()

#define MI_CPPTEST_FIXTURE_TEST_CASE(x, fixture)                        \
    static void tag_##x() { }                                           \
    template<void (*F)()> struct x : public fixture { void run_test(); }; \
    static void run_##x() { x< tag_##x > test; test.run_test(); }       \
    static bool test4fimex_registered_##x = miutil::cpptest::register_test(#x, run_##x); \
    template<void (*F)()> void x<F>::run_test()

#define MI_CPPTEST_TEST_SUITE(x) // nothing
#define MI_CPPTEST_TEST_SUITE_END() // nothing

#define MI_CPPTEST_REQUIRE(x) \
    miutil::cpptest::ensure(static_cast<bool>(x), __FILE__, __LINE__)
#define MI_CPPTEST_CHECK(x) \
    MI_CPPTEST_REQUIRE(x)

#define MI_CPPTEST_FAIL(m) \
    MI_CPPTEST_REQUIRE(false)

#define MI_CPPTEST_REQUIRE_MESSAGE(x, m) \
    MI_CPPTEST_REQUIRE(x)
#define MI_CPPTEST_CHECK_MESSAGE(x, m) \
    MI_CPPTEST_REQUIRE(x)

#define MI_CPPTEST_REQUIRE_EQ(x, y) \
    MI_CPPTEST_REQUIRE(x == y)
#define MI_CPPTEST_CHECK_EQ(x, y) \
    MI_CPPTEST_REQUIRE_EQ(x, y)

#define MI_CPPTEST_CHECK_NE(x, y) \
    MI_CPPTEST_REQUIRE(x != y)
#define MI_CPPTEST_REQUIRE_GT(a, b) \
    MI_CPPTEST_REQUIRE(a > b)
#define MI_CPPTEST_REQUIRE_GE(a, b) \
    MI_CPPTEST_REQUIRE(a >= b)
#define MI_CPPTEST_REQUIRE_LE(a, b) \
    MI_CPPTEST_REQUIRE(a <= b)

#define MI_CPPTEST_REQUIRE_CLOSE(x, y, z) \
    MI_CPPTEST_REQUIRE(miutil::cpptest::check_close(x, y, z))
#define MI_CPPTEST_CHECK_CLOSE(x, y, z) \
    MI_CPPTEST_REQUIRE_CLOSE(x, y, z)

#define MI_CPPTEST_CHECK_THROW(x, ex) \
    do { try { x; } catch (ex&) { break; } MI_CPPTEST_FAIL(); } while(0)
#define MI_CPPTEST_CHECK_NO_THROW(x) \
    do { try { x; } catch (...) { MI_CPPTEST_FAIL(); } } while(0)

#endif // MI_CPPTEST_H
