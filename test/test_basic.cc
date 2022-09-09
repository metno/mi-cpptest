/*
  mi-cpptest

  Copyright (C) 2021 met.no

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


#include "mi_cpptest.h"

#include <map>
#include <ostream>
#include <sstream>

#if 0
MI_CPPTEST_TEST_CASE(test_close)
{
    try {
        const double a=1, b=4, tol=1;
        MI_CPPTEST_REQUIRE_CLOSE(a, b, tol);
    } catch (miutil::cpptest::test_failure&) {
        return; // ok
    }
    MI_CPPTEST_FAIL("test must fail");
}

MI_CPPTEST_TEST_CASE(test_vector)
{
    const std::vector<std::string> a{"1"}, b{"1", "2"};
    MI_CPPTEST_CHECK_EQ(a, b);
}
#endif

MI_CPPTEST_TEST_CASE(test_relations) {
  MI_CPPTEST_REQUIRE_EQ(1, 1);
  MI_CPPTEST_REQUIRE_NE(1, 2);

  MI_CPPTEST_REQUIRE_GE(1, 1);
  MI_CPPTEST_REQUIRE_GT(2, 1);
  MI_CPPTEST_REQUIRE_LE(1, 2);
}

MI_CPPTEST_TEST_CASE(test_map) {
  const std::map<int, int> mapp{{1, 2}, {3, 4}};
  std::ostringstream out;
  out << miutil::cpptest::stringify(mapp);
  MI_CPPTEST_REQUIRE_EQ(out.str(), "{<1,2>,<3,4>}");
}

namespace {
struct NoOstream {
  int n;
};
} // namespace

MI_CPPTEST_TEST_CASE(test_type_without_ostream) {
  std::ostringstream oact;
  oact << miutil::cpptest::stringify(NoOstream{});
  const std::string act = oact.str();

  std::ostringstream exp;
  miutil::cpptest::mi_cpptest_stringify_missing(exp);

  MI_CPPTEST_REQUIRE_EQ(act.c_str(), exp.str());
}

namespace {
struct Stringify {
  int n;
};
void mi_cpptest_stringify(std::ostream &out, const Stringify &) {
  out << "stringify";
}
} // namespace

MI_CPPTEST_TEST_CASE(test_type_with_stringify) {
  std::ostringstream oact;
  oact << miutil::cpptest::stringify(Stringify{7});
  const std::string act = oact.str();

  MI_CPPTEST_REQUIRE_EQ(act, "stringify");
}

namespace {
struct OstreamAndStringify {
  int n;
};
std::ostream &operator<<(std::ostream &out, const OstreamAndStringify &) {
  return out << "ostream";
}
void mi_cpptest_stringify(std::ostream &out, const OstreamAndStringify &) {
  out << "stringify";
}
} // namespace

MI_CPPTEST_TEST_CASE(test_type_with_ostream_and_stringify) {
  std::ostringstream oact;
  oact << miutil::cpptest::stringify(OstreamAndStringify{7});
  const std::string act = oact.str();

  MI_CPPTEST_REQUIRE_EQ(act, "stringify");
}
