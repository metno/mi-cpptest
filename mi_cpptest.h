/* -*- c++ -*-
  mi-cpptest

  Copyright (C) 2019-2021 met.no

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
#include <sstream>
#include <vector>

namespace miutil {
namespace cpptest {

struct test_failure : public std::exception {};

enum test_status { OK = 0, FAIL, SKIP };

class test_recorder {
public:
  void record(const char *file, int line,
              const std::string &msg = std::string());
  test_status status() const;
  const std::vector<std::string> &messages() const { return messages_; }

  static void set_file_prefix(const std::string &prefix) {
    file_prefix_ = prefix;
  }

private:
  static std::string file_prefix_;
  std::vector<std::string> messages_;
};

class test_fixture {
public:
  virtual void set_up() {}
  virtual void tear_down() {}

  test_recorder *mi_cpptest_recorder;
};

class test_fixture_up_down {
public:
  test_fixture_up_down(test_fixture &f) : fixture_(f) { fixture_.set_up(); }
  ~test_fixture_up_down() { fixture_.tear_down(); }

private:
  test_fixture &fixture_;
};

typedef void (*test_function_t)(test_recorder *);

bool register_test(const char* name, test_function_t tf);

bool run_tests(size_t npatterns, char* patterns[]);
bool run_tests_with_prefix(int argc, char *args[]);

bool check_close(double a, double b, double tol);

} // namespace cpptest
} // namespace miutil

#define MI_CPPTEST_TEST_CASE(x)                                                \
  static void x(miutil::cpptest::test_recorder *);                             \
  static bool test4fimex_registered_##x =                                      \
      miutil::cpptest::register_test(#x, x);                                   \
  static void x(miutil::cpptest::test_recorder                                 \
                    *mi_cpptest_recorder) // { test body } after macro

#define MI_CPPTEST_FIXTURE_TEST_CASE(x, fixture)                               \
  static void tag_##x() {}                                                     \
  template <void (*F)()> struct x : public fixture { void run(); };            \
  static void run_##x(miutil::cpptest::test_recorder *tr) {                    \
    x<tag_##x> tf;                                                             \
    tf.mi_cpptest_recorder = tr;                                               \
    miutil::cpptest::test_fixture_up_down tear_down(tf);                       \
    tf.run();                                                                  \
  }                                                                            \
  static bool test4fimex_registered_##x =                                      \
      miutil::cpptest::register_test(#x, run_##x);                             \
  template <void (*F)()> void x<F>::run() // { test body } after macro

#define MI_CPPTEST_TEST_SUITE(x) // nothing
#define MI_CPPTEST_TEST_SUITE_END() // nothing

#define MI_CPPTEST_RECORD(fatal, x, m)                                         \
  do {                                                                         \
    if (!static_cast<bool>(x)) {                                               \
      std::ostringstream msg;                                                  \
      msg << m;                                                                \
      mi_cpptest_recorder->record(__FILE__, __LINE__, msg.str());              \
      if (fatal)                                                               \
        throw miutil::cpptest::test_failure();                                 \
    }                                                                          \
  } while (false)

#define MI_CPPTEST_REQUIRE_MESSAGE(x, m) MI_CPPTEST_RECORD(true, x, m)
#define MI_CPPTEST_REQUIRE(x) MI_CPPTEST_REQUIRE_MESSAGE(x, "")

#define MI_CPPTEST_CHECK_MESSAGE(x, m) MI_CPPTEST_RECORD(false, x, m)
#define MI_CPPTEST_CHECK(x) MI_CPPTEST_CHECK_MESSAGE(x, "")

#define MI_CPPTEST_FAIL(m) \
    MI_CPPTEST_REQUIRE(false)

#define MI_CPPTEST_REQUIRE_EQ(x, y) \
    MI_CPPTEST_REQUIRE(x == y)
#define MI_CPPTEST_CHECK_EQ(x, y) MI_CPPTEST_CHECK(x == y)

#define MI_CPPTEST_CHECK_NE(x, y) MI_CPPTEST_CHECK(x != y)
#define MI_CPPTEST_REQUIRE_GT(a, b) \
    MI_CPPTEST_REQUIRE(a > b)
#define MI_CPPTEST_REQUIRE_GE(a, b) \
    MI_CPPTEST_REQUIRE(a >= b)
#define MI_CPPTEST_REQUIRE_LE(a, b) \
    MI_CPPTEST_REQUIRE(a <= b)

#define MI_CPPTEST_REQUIRE_CLOSE(x, y, z) \
    MI_CPPTEST_REQUIRE(miutil::cpptest::check_close(x, y, z))
#define MI_CPPTEST_CHECK_CLOSE(x, y, z)                                        \
  MI_CPPTEST_CHECK(miutil::cpptest::check_close(x, y, z))

#define MI_CPPTEST_CHECK_THROW(x, ex) \
    do { try { x; } catch (ex&) { break; } MI_CPPTEST_FAIL(); } while(0)
#define MI_CPPTEST_CHECK_NO_THROW(x) \
    do { try { x; } catch (...) { MI_CPPTEST_FAIL(); } } while(0)

#endif // MI_CPPTEST_H
