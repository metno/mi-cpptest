/*
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

#include "mi_cpptest.h"

#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <sstream>

namespace {

typedef void (*test_function_t)();
struct registered_test {
    std::string name;
    test_function_t test;
    void operator()() const { test(); }
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

std::string yaml_escape(const std::string& text)
{
    std::ostringstream escaped;
    for (char ch : text) {
        if (ch < ' ' && ch != '\n' && ch != 9) {
            unsigned int u = (unsigned int)ch;
            escaped << "\\x" << hexchar(u >> 4) << hexchar(u);
            continue;
        }
        if (ch == '"' || ch == '\\')
            escaped << '\\';
        escaped << ch;
    }
    return escaped.str();
}

} // namespace

namespace miutil {
namespace cpptest {

typedef std::vector<registered_test> registered_test_v;

registered_test_v& registered_tests()
{
    static registered_test_v tests;
    return tests;
}

bool check_close(double a, double b, double tol)
{
    if (a == b)
        return true;
    const double d = std::abs(a - b), aa = std::abs(a), bb = std::abs(b);
    return (d <= aa*tol) && (d <= bb*tol);
}

void ensure(bool b, const char* file, int lineno)
{
    if (!b)
        throw test_failure(file, lineno);
}

bool register_test(const char* name, test_function_t tf)
{
    registered_tests().push_back(registered_test {name, tf});
    return true;
}

enum TestStatus {
    OK = 0,
    FAIL,
    SKIP
};

void write_test_status(std::ostream& out, TestStatus status, size_t number, const registered_test& rt, const std::string& message)
{
    if (status == FAIL)
        out << "not ";
    out << "ok " << number;
    out << ' ' << rt.name;
    if (status == SKIP)
        out << " # SKIP";
    out << std::endl;
    if (!message.empty()) {
        out << " ---" << std::endl
            << " message: \"" << yaml_escape(message) << '"' << std::endl
            << " ..." << std::endl;
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

        TestStatus status = use_default ? FAIL : SKIP;
        std::string message;

        std::smatch filter_match;
        for (const test_filter& tf : filters) {
            if (std::regex_match(rt.name, filter_match, tf.regex)) {
                status = tf.exclusive ? SKIP : FAIL;
                break;
            }
        }

        if (status != SKIP) {
            try {
                rt();
                status = OK;
            } catch (const test_failure& tf) {
                std::ostringstream msg;
                msg << "failed in " << tf.file() << ":" << tf.lineno();
                message = msg.str();
            } catch (std::exception& e) {
                message = "uncaught exception: " + std::string(e.what());
            } catch (...) {
                message = "uncaught exception";
            }
            if (status != OK)
                all_passed = false;
        }
        write_test_status(std::cout, status, test_number, rt, message);
    }
    return all_passed;
}

} // namespace cpptest
} // namespace miutil
