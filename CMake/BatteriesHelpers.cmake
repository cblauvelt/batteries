#
# Copyright 2019 The Batteries Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This CMAKE file was originally taken from the Abseil project.
include(CMakeParseArguments)
include(BatteriesConfigureCopts)
include(BatteriesInstallDirs)

# The IDE folder for Batteries that will be used if Batteries is included in a CMake
# project that sets
#    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# For example, Visual Studio supports folders.
set(BATT_IDE_FOLDER Batteries)

# batteries_cc_library()
#
# CMake function to imitate Bazel's cc_library rule.
#
# Parameters:
# NAME: name of target (see Note)
# HDRS: List of public header files for the library
# SRCS: List of source files for the library
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
# PUBLIC: Add this so that this library will be exported under batteries::
# Also in IDE, target will appear in Batteries folder while non PUBLIC will be in Batteries/internal.
# TESTONLY: When added, this target will only be built if user passes -DBATT_RUN_TESTS=ON to CMake.
#
# Note:
# By default, batteries_cc_library will always create a library named batteries_${NAME},
# and alias target batteries::${NAME}.  The batteries:: form should always be used.
# This is to reduce namespace pollution.
#
# batteries_cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
# )
# batteries_cc_library(
#   NAME
#     fantastic_lib
#   SRCS
#     "b.cc"
#   DEPS
#     batteries::awesome # not "awesome" !
#   PUBLIC
# )
#
# batteries_cc_library(
#   NAME
#     main_lib
#   ...
#   DEPS
#     batteries::fantastic_lib
# )
#
# TODO: Implement "ALWAYSLINK"
function(batteries_cc_library)
  cmake_parse_arguments(BATT_CC_LIB
    "DISABLE_INSTALL;PUBLIC;TESTONLY"
    "NAME"
    "HDRS;SRCS;COPTS;DEFINES;LINKOPTS;DEPS"
    ${ARGN}
  )

  if(NOT BATT_CC_LIB_TESTONLY OR BATT_RUN_TESTS)
    if(BATT_ENABLE_INSTALL)
      set(_NAME "${BATT_CC_LIB_NAME}")
    else()
      set(_NAME "batteries_${BATT_CC_LIB_NAME}")
    endif()

    # Check if this is a header-only library
    # Note that as of February 2019, many popular OS's (for example, Ubuntu
    # 16.04 LTS) only come with cmake 3.5 by default.  For this reason, we can't
    # use list(FILTER...)
    set(BATT_CC_SRCS "${BATT_CC_LIB_SRCS}")
    foreach(src_file IN LISTS BATT_CC_SRCS)
      if(${src_file} MATCHES ".*\\.(h|inc)")
        list(REMOVE_ITEM BATT_CC_SRCS "${src_file}")
      endif()
    endforeach()
    if("${BATT_CC_SRCS}" STREQUAL "")
      set(BATT_CC_LIB_IS_INTERFACE 1)
    else()
      set(BATT_CC_LIB_IS_INTERFACE 0)
    endif()

    if(NOT BATT_CC_LIB_IS_INTERFACE)
      # CMake creates static libraries by default. Users can specify
      # -DBUILD_SHARED_LIBS=ON during initial configuration to build shared
      # libraries instead.
      add_library(${_NAME} "")
      target_sources(${_NAME} PRIVATE ${BATT_CC_LIB_SRCS} ${BATT_CC_LIB_HDRS})
      target_include_directories(${_NAME}
        PUBLIC
          "$<BUILD_INTERFACE:${BATT_COMMON_INCLUDE_DIRS}>"
          $<INSTALL_INTERFACE:${BATT_INSTALL_INCLUDEDIR}>
      )
      target_compile_options(${_NAME}
        PRIVATE ${BATT_CC_LIB_COPTS})
      target_link_libraries(${_NAME}
        PUBLIC ${BATT_CC_LIB_DEPS}
        PRIVATE
          ${BATT_CC_LIB_LINKOPTS}
          ${BATT_DEFAULT_LINKOPTS}
      )
      target_compile_definitions(${_NAME} PUBLIC ${BATT_CC_LIB_DEFINES})

      # Add all Batteries targets to a a folder in the IDE for organization.
      if(BATT_CC_LIB_PUBLIC)
        set_property(TARGET ${_NAME} PROPERTY FOLDER ${BATT_IDE_FOLDER})
      elseif(BATT_CC_LIB_TESTONLY)
        set_property(TARGET ${_NAME} PROPERTY FOLDER ${BATT_IDE_FOLDER}/test)
      else()
        set_property(TARGET ${_NAME} PROPERTY FOLDER ${BATT_IDE_FOLDER}/internal)
      endif()

      # INTERFACE libraries can't have the CXX_STANDARD property set
      set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD ${BATT_CXX_STANDARD})
      set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

      # When being installed, we lose the batteries_ prefix.  We want to put it back
      # to have properly named lib files.  This is a no-op when we are not being
      # installed.
      set_target_properties(${_NAME} PROPERTIES
        OUTPUT_NAME "batteries_${_NAME}"
      )
    else()
      # Generating header-only library
      add_library(${_NAME} INTERFACE)
      target_include_directories(${_NAME}
        INTERFACE
          "$<BUILD_INTERFACE:${BATT_COMMON_INCLUDE_DIRS}>"
          $<INSTALL_INTERFACE:${BATT_INSTALL_INCLUDEDIR}>
        )
      target_link_libraries(${_NAME}
        INTERFACE
          ${BATT_CC_LIB_DEPS}
          ${BATT_CC_LIB_LINKOPTS}
          ${BATT_DEFAULT_LINKOPTS}
      )
      target_compile_definitions(${_NAME} INTERFACE ${BATT_CC_LIB_DEFINES})
    endif()

    # TODO currently we don't install googletest alongside batteries sources, so
    # installed batteries can't be tested.
    if(NOT BATT_CC_LIB_TESTONLY AND BATT_ENABLE_INSTALL)
      install(TARGETS ${_NAME} EXPORT ${PROJECT_NAME}Targets
            RUNTIME DESTINATION ${BATT_INSTALL_BINDIR}
            LIBRARY DESTINATION ${BATT_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${BATT_INSTALL_LIBDIR}
      )
    endif()

    add_library(batteries::${BATT_CC_LIB_NAME} ALIAS ${_NAME})
  endif()
endfunction()

# batteries_cc_test()
#
# CMake function to imitate Bazel's cc_test rule.
#
# Parameters:
# NAME: name of target (see Usage below)
# SRCS: List of source files for the binary
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
#
# Note:
# By default, batteries_cc_test will always create a binary named batteries_${NAME}.
# This will also add it to ctest list as batteries_${NAME}.
#
# Usage:
# batteries_cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
#   PUBLIC
# )
#
# batteries_cc_test(
#   NAME
#     awesome_test
#   SRCS
#     "awesome_test.cc"
#   DEPS
#     batteries::awesome
#     gmock
#     gtest_main
# )
function(batteries_cc_test)
  if(NOT BATT_RUN_TESTS)
    return()
  endif()

  cmake_parse_arguments(BATT_CC_TEST
    ""
    "NAME"
    "SRCS;COPTS;DEFINES;LINKOPTS;DEPS"
    ${ARGN}
  )

  set(_NAME "batteries_${BATT_CC_TEST_NAME}")
  add_executable(${_NAME} "")
  target_sources(${_NAME} PRIVATE ${BATT_CC_TEST_SRCS})
  target_include_directories(${_NAME}
    PUBLIC ${BATT_COMMON_INCLUDE_DIRS}
    PRIVATE ${CONAN_INCLUDE_DIRS_GTEST}
  )
  target_compile_definitions(${_NAME}
    PUBLIC ${BATT_CC_TEST_DEFINES}
  )
  target_compile_options(${_NAME}
    PRIVATE ${BATT_CC_TEST_COPTS}
  )
  target_link_libraries(${_NAME}
    PUBLIC ${BATT_CC_TEST_DEPS}
    PRIVATE ${BATT_CC_TEST_LINKOPTS}
  )
  # Add all Batteries targets to a a folder in the IDE for organization.
  set_property(TARGET ${_NAME} PROPERTY FOLDER ${BATT_IDE_FOLDER}/test)

  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD ${BATT_CXX_STANDARD})
  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

  add_test(NAME ${_NAME} COMMAND ${_NAME})
endfunction()


function(check_target my_target)
  if(NOT TARGET ${my_target})
    message(FATAL_ERROR " BATT: compiling batteries requires a ${my_target} CMake target in your project,
                   see CMake/README.md for more details")
  endif(NOT TARGET ${my_target})
endfunction()