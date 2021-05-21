# mi-cpptest
#
# Copyright (C) 2019-2021 met.no
#
# Contact information:
# Norwegian Meteorological Institute
# Box 43 Blindern
# 0313 OSLO
# NORWAY
# email: diana@met.no
#
# This file is part of mi-cpptest.
#
# mi-cpptest is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# mi-cpptest is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mi-cpptest; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

PROJECT(mi-cpptest LANGUAGES CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

SET(MI_CPPTEST_MASTER_PROJECT OFF)
IF(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  SET(MI_CPPTEST_MASTER_PROJECT ON)
ENDIF()

INCLUDE(CMakePackageConfigHelpers)
INCLUDE(GNUInstallDirs)
SET(CMAKE_CXX_STANDARD 11)

SET(MI_CPPTEST_HEADERS
  mi_cpptest.h
  mi_cpptest_version.h
)

IF(NOT MI_CPPTEST_MASTER_PROJECT)
  MESSAGE(STATUS "Add mi-cpptest subdirectory from ${CMAKE_CURRENT_LIST_DIR}")
ENDIF()

ADD_LIBRARY(mi-cpptest STATIC
  mi_cpptest.cc
  ${MI_CPPTEST_HEADERS}
)

SET(MI_CPPTEST_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

TARGET_INCLUDE_DIRECTORIES(mi-cpptest
  PUBLIC
  $<BUILD_INTERFACE:${MI_CPPTEST_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

ADD_LIBRARY(mi-cpptest-main STATIC
  mi_cpptest_main.cc
)

TARGET_LINK_LIBRARIES(mi-cpptest-main
  PUBLIC
  mi-cpptest
)

IF(MI_CPPTEST_MASTER_PROJECT)
  FILE(STRINGS "mi_cpptest_version.h" version_defines
    REGEX "#define .*_VERSION_(MAJOR|MINOR|PATCH) ")
  FOREACH(ver ${version_defines})
    IF (ver MATCHES "#define .*_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
      SET(MI_CPPTEST_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    ENDIF()
  ENDFOREACH()

  ENABLE_TESTING()
  ADD_SUBDIRECTORY(test)

  INSTALL(
    TARGETS mi-cpptest mi-cpptest-main
    EXPORT mi-cpptest
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  INSTALL(
    FILES ${MI_CPPTEST_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  SET(MI_CPPTEST_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/mi-cpptest")

  INSTALL(
    EXPORT mi-cpptest
    DESTINATION "${MI_CPPTEST_CMAKE_DIR}"
    FILE mi-cpptest-targets.cmake
  )

  WRITE_BASIC_PACKAGE_VERSION_FILE(
    "${CMAKE_CURRENT_BINARY_DIR}/mi-cpptest-config-version.cmake"
    VERSION
    "${MI_CPPTEST_VERSION_MAJOR}.${MI_CPPTEST_VERSION_MINOR}.${MI_CPPTEST_VERSION_PATCH}"
    COMPATIBILITY
    SameMajorVersion
  )

  INSTALL(
    FILES mi-cpptest-config.cmake "${CMAKE_CURRENT_BINARY_DIR}/mi-cpptest-config-version.cmake"
    DESTINATION "${MI_CPPTEST_CMAKE_DIR}"
  )
ENDIF(MI_CPPTEST_MASTER_PROJECT)
