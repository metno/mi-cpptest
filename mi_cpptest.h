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
#include <functional>
#include <memory>
#include <ostream>
#include <sstream>
#include <type_traits>
#include <vector>

namespace miutil {
namespace cpptest {

// from https://en.cppreference.com/w/cpp/types/void_t
template <typename T, typename = void>
struct is_iterable : std::false_type {};
template <typename T>
struct is_iterable<T, std::void_t<decltype(std::declval<T>().begin()),
                                  decltype(std::declval<T>().end())>>
    : std::true_type {};

// from https://codereview.stackexchange.com/questions/250619/using-sfinae-to-provide-default-output-operators
template <typename T, typename = void>
struct has_output_operator : std::false_type {};
template <typename T>
struct has_output_operator<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};

void mi_cpptest_stringify_missing(std::ostream &out);

template <typename T, typename = void>
struct has_mi_cpptest_stringify : std::false_type {};
template <typename T>
struct has_mi_cpptest_stringify<T, std::void_t<decltype(mi_cpptest_stringify(std::declval<std::ostream&>(), std::declval<T>()))>> : std::true_type {};

template <class C, typename = void>
struct stringifier {
  stringifier(const C& c) : c_(c) {}
  void write(std::ostream &out) const { mi_cpptest_stringify_missing(out); }
  const C &c_;
};

template <class C>
stringifier<C> stringify(const C &c)
{
  return stringifier<C>(c);
}

template <class C>
std::ostream &operator<<(std::ostream &out, const stringifier<C> &s) {
  s.write(out);
  return out;
}

template <class C>
struct stringifier<C, typename std::enable_if<has_mi_cpptest_stringify<C>::value>::type> {
  stringifier(const C &c) : c_(c) {}
  void write(std::ostream &out) const { mi_cpptest_stringify(out, c_); }
  const C &c_;
};

template <class C>
struct stringifier<C, typename std::enable_if<has_output_operator<C>::value && !has_mi_cpptest_stringify<C>::value>::type> {
  stringifier(const C &c) : c_(c) {}
  void write(std::ostream &out) const { out << c_; }
  const C &c_;
};

template <class C>
struct stringifier<C, typename std::enable_if<is_iterable<C>::value && !has_output_operator<C>::value && !has_mi_cpptest_stringify<C>::value>::type> {
  stringifier(const C &c) : c_(c) {}
  void write(std::ostream &out) const {
    char sep = '{';
    for (const auto &e : c_) {
      out << sep << stringify(e);
      sep = ',';
    }
    out << '}';
  }
  const C &c_;
};

template <class A, class B>
struct stringifier<std::pair<A, B>> {
  typedef typename std::pair<A, B> P;
  stringifier(const P &p) : p_(p) {}
  void write(std::ostream &out) const {
    out << '<' << stringify(p_.first) << ',' << stringify(p_.second) << '>';
  }
  const P &p_;
};

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

template<class A>
struct equal_to {
    bool operator()(const A& a, const A&b) const { return std::equal_to<A>()(a, b); }
};

template<>
struct equal_to<const char* const> {
    bool operator()(const std::string& a, const std::string& b) const { return a == b; }
};

template<class A>
struct not_equal_to {
    bool operator()(const A& a, const A&b) const { return !equal_to<A>()(a, b); }
};

template <class C> struct is_close {
  bool operator()(const C &a, const C &b, const C &tol) const {
    if (a == b)
      return true;
    const C d = std::abs(a - b), aa = std::abs(a), bb = std::abs(b);
    return (d <= aa * tol) && (d <= bb * tol);
  }
};

template <class C> struct is_near {
  bool operator()(const C &a, const C &b, const C &diff) const {
    return std::abs(a - b) < diff;
  }
};

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

#define MI_CPPTEST___RECORD(fatal, x, m)                                       \
  do {                                                                         \
    if (!static_cast<bool>(x)) {                                               \
      std::ostringstream msg;                                                  \
      msg << m;                                                                \
      mi_cpptest_recorder->record(__FILE__, __LINE__, msg.str());              \
      if (fatal)                                                               \
        throw miutil::cpptest::test_failure();                                 \
    }                                                                          \
  } while (false)

#define MI_CPPTEST___RECORD_BOOL(fatal, x)                                     \
  MI_CPPTEST___RECORD(fatal, x, "'" #x "' does not evaluate to true")

#define MI_CPPTEST_REQUIRE_MESSAGE(x, m) MI_CPPTEST___RECORD(true, x, m)
#define MI_CPPTEST_REQUIRE(x) MI_CPPTEST___RECORD_BOOL(true, x)

#define MI_CPPTEST_CHECK_MESSAGE(x, m) MI_CPPTEST___RECORD(false, x, m)
#define MI_CPPTEST_CHECK(x) MI_CPPTEST___RECORD_BOOL(false, x)

#define MI_CPPTEST_FAIL(m) MI_CPPTEST___RECORD(true, false, "")

#define MI_CPPTEST___RECORD_OP2(fatal, x, y, op)                               \
  do {                                                                         \
    const auto xx = (x);                                                       \
    const auto yy = (y);                                                       \
    const auto oper = op<decltype(xx)>{};                                      \
    MI_CPPTEST___RECORD(fatal, oper(xx, yy),                                   \
                        #op " failed for " #x "="                              \
                            << miutil::cpptest::stringify(xx)                  \
                            << " and " #y "="                                  \
                            << miutil::cpptest::stringify(yy));                \
  } while (0)
#define MI_CPPTEST_REQUIRE_EQ(x, y)                                            \
  MI_CPPTEST___RECORD_OP2(true, x, y, miutil::cpptest::equal_to)
#define MI_CPPTEST_CHECK_EQ(x, y)                                              \
  MI_CPPTEST___RECORD_OP2(false, x, y, miutil::cpptest::equal_to)
#define MI_CPPTEST_REQUIRE_NE(x, y)                                            \
  MI_CPPTEST___RECORD_OP2(true, x, y, miutil::cpptest::not_equal_to)
#define MI_CPPTEST_CHECK_NE(x, y)                                              \
  MI_CPPTEST___RECORD_OP2(false, x, y, miutil::cpptest::not_equal_to)
#define MI_CPPTEST_REQUIRE_GT(x, y)                                            \
  MI_CPPTEST___RECORD_OP2(true, x, y, std::greater)
#define MI_CPPTEST_CHECK_GT(x, y)                                              \
  MI_CPPTEST___RECORD_OP2(false, x, y, std::greater)
#define MI_CPPTEST_REQUIRE_GE(x, y)                                            \
  MI_CPPTEST___RECORD_OP2(true, x, y, std::greater_equal)
#define MI_CPPTEST_CHECK_GE(x, y)                                              \
  MI_CPPTEST___RECORD_OP2(false, x, y, std::greater_equal)
#define MI_CPPTEST_REQUIRE_LT(x, y)                                            \
  MI_CPPTEST___RECORD_OP2(true, x, y, std::less)
#define MI_CPPTEST_CHECK_LT(x, y)                                              \
  MI_CPPTEST___RECORD_OP2(false, x, y, std::less)
#define MI_CPPTEST_REQUIRE_LE(x, y)                                            \
  MI_CPPTEST___RECORD_OP2(true, x, y, std::less_equal)
#define MI_CPPTEST_CHECK_LE(x, y)                                              \
  MI_CPPTEST___RECORD_OP2(false, x, y, std::less_equal)

#define MI_CPPTEST___RECORD_OP3(fatal, x, y, z, op)                            \
  do {                                                                         \
    const auto xx = (x);                                                       \
    const auto yy = (y);                                                       \
    const auto zz = (z);                                                       \
    const auto oper = op<decltype(xx)>{};                                      \
    MI_CPPTEST___RECORD(                                                       \
        fatal, oper(xx, yy, zz),                                               \
        #op " failed for " #x "="                                              \
            << miutil::cpptest::stringify(xx) << " and " #y "="                \
            << miutil::cpptest::stringify(yy) << " and " #z "="                \
            << miutil::cpptest::stringify(zz));                                \
  } while (0)

#define MI_CPPTEST___RECORD_CLOSE(fatal, x, y, z)                              \
  do {                                                                         \
    const auto xx = (x);                                                       \
    const auto yy = (y);                                                       \
    const auto zz = (z);                                                       \
    MI_CPPTEST___RECORD(fatal, miutil::cpptest::is_close(xx, yy, zz),          \
                        #x "='" << miutil::cpptest::stringify(xx)              \
                                << "' is not within tolerance " #z "='"        \
                                << miutil::cpptest::stringify(zz)              \
                                << "' from " #y "='"                           \
                                << miutil::cpptest::stringify(yy) << "'");     \
  } while (0)
#define MI_CPPTEST_REQUIRE_CLOSE(x, y, z)                                      \
  MI_CPPTEST___RECORD_OP3(true, x, y, z, miutil::cpptest::is_close)
#define MI_CPPTEST_CHECK_CLOSE(x, y, z)                                        \
  MI_CPPTEST___RECORD_OP3(false, x, y, z, miutil::cpptest::is_close)

#define MI_CPPTEST_REQUIRE_NEAR(x, y, z)                                       \
  MI_CPPTEST___RECORD_OP3(true, x, y, z, miutil::cpptest::is_near)
#define MI_CPPTEST_CHECK_NEAR(x, y, z)                                         \
  MI_CPPTEST___RECORD_OP3(false, x, y, z, miutil::cpptest::is_near)

#define MI_CPPTEST_CHECK_THROW(x, ex) \
    do { try { x; } catch (ex&) { break; } MI_CPPTEST_FAIL(); } while(0)
#define MI_CPPTEST_CHECK_NO_THROW(x) \
    do { try { x; } catch (...) { MI_CPPTEST_FAIL(); } } while(0)

#endif // MI_CPPTEST_H
