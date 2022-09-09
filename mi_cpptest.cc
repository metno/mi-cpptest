/*
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

#include "mi_cpptest.h"

#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <sstream>

namespace {

struct registered_test {
    std::string name;
    miutil::cpptest::test_function_t test;
    void operator()(miutil::cpptest::test_recorder *tr) const { test(tr); }
};

struct test_filter {
    bool exclusive;
    std::regex regex;
};

char hexchar(unsigned int i)
{
    const char hexchars[17] = "0123456789ABCDEF";
    return hexchars[i & 0xF];
}

void yaml_escaped_block(std::ostream &out, const std::string &text,
                        const std::string &indent) {
  out << indent;
  for (char ch : text) {
    if (ch == '\n') {
      out << '\n' << indent;
    } else if (ch < ' ' && ch != 9) {
      unsigned int u = (unsigned int)ch;
      out << "\\x" << hexchar(u >> 4) << hexchar(u);
    } else if (ch == '"' || ch == '\\') {
      out << '\\' << ch;
    } else {
      out << ch;
    }
  }
  out << '\n';
}

std::string strip_common_prefix(const std::string &from,
                                const std::string &prefix,
                                const std::string &strip) {
  size_t common = 0;
  const size_t max = std::min(from.size(), prefix.size());
  while (common < max && from[common] == prefix[common])
    ++common;
  const auto idx = from.find_first_not_of(strip, common);
  if (idx != std::string::npos)
    common = idx;
  return from.substr(common);
}

} // namespace

namespace miutil {
namespace cpptest {

void mi_cpptest_stringify_missing(std::ostream &out) {
  out << "???";
}

std::string test_recorder::file_prefix_;

void test_recorder::record(const char *file, int line,
                           const std::string &note) {
  std::ostringstream msg;
  if (line >= 0) {
    msg << strip_common_prefix(file, file_prefix_, "/:.") << ":" << line;
  }
  if (!note.empty()) {
    if (line >= 0)
      msg << ' ';
    msg << note;
  }
  messages_.push_back(msg.str());
}

test_status test_recorder::status() const {
  return messages_.empty() ? OK : FAIL;
}

typedef std::vector<registered_test> registered_test_v;

registered_test_v& registered_tests()
{
    static registered_test_v tests;
    return tests;
}

bool register_test(const char* name, test_function_t tf)
{
    registered_tests().push_back(registered_test {name, tf});
    return true;
}

void write_test_status(std::ostream &out, test_status status, size_t number,
                       const registered_test &rt, const std::string &message) {
  if (status == FAIL)
    out << "not ";
  out << "ok " << number;
  out << ' ' << rt.name;
  if (status == SKIP)
    out << " # SKIP";
  out << std::endl;
  if (!message.empty()) {
    out << " ---" << std::endl << " message: |" << std::endl;
    yaml_escaped_block(out, message, "   ");
    out << " ..." << std::endl;
  }
}

bool run_tests(size_t npatterns, char* patterns[])
{
    std::vector<test_filter> filters;
    filters.reserve(npatterns);
    size_t n_exclusive = 0;
    for (size_t i=0; i<npatterns; ++i) {
        char* pattern = patterns[i];
        test_filter tf;
        tf.exclusive = (pattern[0] == '-');
        if (tf.exclusive) {
            n_exclusive += 1;
            pattern += 1;
        }
        tf.regex = std::regex(pattern);
        filters.push_back(tf);
    }
    const bool use_default = (filters.empty() || n_exclusive == filters.size());

    std::cout << "TAP version 13" << std::endl
              << "1.." << registered_tests().size() << std::endl;
    bool all_passed = true;
    size_t test_number = 0;
    for (const registered_test& rt : registered_tests()) {
        test_number += 1;

        test_status status = use_default ? FAIL : SKIP;
        std::string message;

        std::smatch filter_match;
        for (const test_filter& tf : filters) {
            if (std::regex_match(rt.name, filter_match, tf.regex)) {
                status = tf.exclusive ? SKIP : FAIL;
                break;
            }
        }

        if (status != SKIP) {
          test_recorder tr;
          try {
            rt(&tr);
          } catch (const test_failure &tf) {
            // recorded in test_recorder::fail
            // tr.record(tf.file(), tf.line(), tf.message());
          } catch (std::exception &e) {
            tr.record("", -1, "uncaught exception: " + std::string(e.what()));
          } catch (...) {
            tr.record("", -1, "uncaught exception");
          }
          status = tr.status();
          if (status != OK) {
            all_passed = false;

            std::ostringstream msg;
            bool first = true;
            for (const auto &m : tr.messages()) {
              if (!first)
                msg << '\n';
              msg << m;
              first = false;
            }
            message = msg.str();
          }
        }
        write_test_status(std::cout, status, test_number, rt, message);
    }
    return all_passed;
}

bool run_tests_with_prefix(int argc, char *args[]) {
  test_recorder::set_file_prefix(args[0]);
  return run_tests(argc - 1, args + 1);
}

} // namespace cpptest
} // namespace miutil
