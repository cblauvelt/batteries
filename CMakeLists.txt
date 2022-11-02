#
# Copyright 2019 The Batteries Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#			https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#####################################
# Define Project-Wide Settings
#####################################
cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

# Enables the Visibility Property on all target types
cmake_policy(SET CMP0063 NEW)
# Enables the MSVC_RUNTIME_LIBRARY property on targets
cmake_policy(SET CMP0091 NEW)
# Compiler id for Apple Clang is now AppleClang.
cmake_policy(SET CMP0025 NEW)
# if command can use IN_LIST
cmake_policy(SET CMP0057 NEW)
# Project version variables are the empty std::string if version is unspecified
cmake_policy(SET CMP0048 NEW)
# Define the Project Name and Description
project(batteries LANGUAGES CXX DESCRIPTION "batteries a library to get you started")

# Set Project version (used for library versioning and for CPack)
set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 9)
set(CMAKE_PROJECT_VERSION_PATCH 1)
set(CMAKE_PROJECT_VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})

# Set required C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
# Generate the compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
# Set the Relative Path Configurations
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# Configure the visibility of symbols in targets
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
# Define installation folder and the module path (this one required by conan)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

#####################################
# Define Options
#####################################
option(BATT_BUILD_TESTING "Build tests" FALSE)
option(BUILD_SHARED_LIBS "Build shared libraries" FALSE)
option(BUILD_WITH_MT "Build libraries as MultiThreaded DLL (Windows Only)" FALSE)

#####################################
# Define CMake Module Imports
#####################################

list(APPEND CMAKE_MODULE_PATH
${CMAKE_CURRENT_LIST_DIR}/cmake
${CMAKE_CURRENT_LIST_DIR}/batteries/copts
)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(InstallRequiredSystemLibraries)
include(GenerateExportHeader)
include(BatteriesInstallDirs)
include(BatteriesHelpers)
include(clang_format)
include(clang_format)
include(compiler_options)
include(cpack)
include(cppcheck)
include(dependencies)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

include(CheckCXXCompilerFlag)

# if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#     include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#     conan_basic_setup()
# else()
#     message(FATAL_ERROR "The file conanbuildinfo.cmake could not be generated")
# endif()

# when batteries is included as subproject (i.e. using add_subdirectory(abseil-cpp))
# in the source tree of a project that uses it, install rules are disabled.
if(NOT "^${CMAKE_SOURCE_DIR}$" STREQUAL "^${PROJECT_SOURCE_DIR}$")
	set(BATT_ENABLE_INSTALL FALSE)
else()
	set(BATT_ENABLE_INSTALL TRUE)
endif()

##
## Using batteries targets
##
## all public batteries targets are
## exported with the batteries:: prefix
##
## DO NOT rely on the internal targets outside of the prefix


# include current path and dependencies
list(APPEND BATT_COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${absl_INCLUDE_DIRS})
list(APPEND BATT_COMMON_LIBS ${absl_LIBS})

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(BATT_USING_CLANG ON)
else()
	set(BATT_USING_CLANG OFF)
endif()

message("-- Testing is ${BATT_BUILD_TESTING}")
if(${BATT_BUILD_TESTING})
	# enable CTest.	This will set BATT_BUILD_TESTING to ON unless otherwise specified
	# on the command line
    include(CTest)
	enable_testing()
	list(APPEND BATT_COMMON_INCLUDE_DIRS ${GTest_INCLUDE_DIRS})
	list(APPEND BATT_TEST_COMMON_LIBRARIES Threads::Threads ${GTest_LIBS})
endif()

add_subdirectory(batteries)
