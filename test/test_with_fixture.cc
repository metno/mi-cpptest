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

namespace {
int up_ = 0, down_ = 0;

struct fixture : miutil::cpptest::test_fixture {
  void set_up() override { up_ += 1; }
  void tear_down() override { down_ += 1; }
};
} // namespace

MI_CPPTEST_FIXTURE_TEST_CASE(test_fixture, fixture) {
  MI_CPPTEST_CHECK_EQ(1, up_);
  MI_CPPTEST_CHECK_EQ(0, down_);
}

MI_CPPTEST_TEST_CASE(test_after_fixture) {
  MI_CPPTEST_CHECK_EQ(1, up_);
  MI_CPPTEST_CHECK_EQ(1, down_);
}
