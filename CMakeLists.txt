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

cmake_minimum_required(VERSION 3.15)

# Compiler id for Apple Clang is now AppleClang.
cmake_policy(SET CMP0025 NEW)

# if command can use IN_LIST
cmake_policy(SET CMP0057 NEW)

# Project version variables are the empty std::string if version is unspecified
cmake_policy(SET CMP0048 NEW)

project(batteries CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)
include(CheckCXXCompilerFlag)
include(conan.cmake)

# integrate conan build process
if(NOT EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_cmake_run(CONANFILE conanfile.py
                    BASIC_SETUP CMAKE_TARGETS
                    BUILD missing)

    conan_cmake_autodetect(settings)

endif()

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(FATAL_ERROR "The file conanbuildinfo.cmake could not be generated")
endif()

# when batteries is included as subproject (i.e. using add_subdirectory(abseil-cpp))
# in the source tree of a project that uses it, install rules are disabled.
if(NOT "^${CMAKE_SOURCE_DIR}$" STREQUAL "^${PROJECT_SOURCE_DIR}$")
	set(BATT_ENABLE_INSTALL FALSE)
else()
	set(BATT_ENABLE_INSTALL TRUE)
endif()

list(APPEND CMAKE_MODULE_PATH
	${CMAKE_CURRENT_LIST_DIR}/CMake
	${CMAKE_CURRENT_LIST_DIR}/batteries/copts
)

include(BatteriesInstallDirs)
include(CMakePackageConfigHelpers)
include(BatteriesHelpers)


##
## Using batteries targets
##
## all public batteries targets are
## exported with the batteries:: prefix
##
## DO NOT rely on the internal targets outside of the prefix


# include current path
list(APPEND BATT_COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(BATT_USING_CLANG ON)
else()
	set(BATT_USING_CLANG OFF)
endif()

# find dependencies
## pthread
find_package(Threads REQUIRED)

option(BATT_RUN_TESTS "If ON, Batteries tests will be run." ON)

if(${BATT_RUN_TESTS})
	# enable CTest.	This will set BUILD_TESTING to ON unless otherwise specified
	# on the command line
	include(CTest)
	enable_testing()
endif()

## check targets
if(BUILD_TESTING)

	list(APPEND BATT_TEST_COMMON_LIBRARIES
		gtest_main
		gtest
		gmock
		${CMAKE_THREAD_LIBS_INIT}
	)
endif()

add_subdirectory(batteries)

# if(BATT_ENABLE_INSTALL)
#	 # batteries:lts-remove-begin(system installation is supported for LTS releases)
#	 # We don't support system-wide installation
#	 list(APPEND SYSTEM_INSTALL_DIRS "/usr/local" "/usr" "/opt/" "/opt/local" "c:/Program Files/${PROJECT_NAME}")
#	 if(NOT DEFINED CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX IN_LIST SYSTEM_INSTALL_DIRS)
#		 message(WARNING "\
#	 The default and system-level install directories are unsupported except in LTS \
#	 releases of Batteries.	Please set CMAKE_INSTALL_PREFIX to install Batteries in your \
#	 source or build tree directly.\
#		 ")
#	 endif()
#	 # batteries:lts-remove-end

#	 # install as a subdirectory only
#	 install(EXPORT ${PROJECT_NAME}Targets
#		 NAMESPACE batteries::
#		 DESTINATION "${BATT_INSTALL_CONFIGDIR}"
#	 )

#	 configure_package_config_file(
#		 CMake/battConfig.cmake.in
#		 "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
#		 INSTALL_DESTINATION "${BATT_INSTALL_CONFIGDIR}"
#	 )
#	 install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
#		 DESTINATION "${BATT_INSTALL_CONFIGDIR}"
#	 )

#	 # Batteries only has a version in LTS releases.	This mechanism is accomplished
#	 # Batteries's internal Copybara (https://github.com/google/copybara) workflows and
#	 # isn't visible in the CMake buildsystem itself.
#	 if(batt_VERSION)
#		 write_basic_package_version_file(
#			 "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
#			 COMPATIBILITY ExactVersion
#		 )

#		 install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
#			 DESTINATION ${BATT_INSTALL_CONFIGDIR}
#		 )
#	 endif()	# batt_VERSION

#	 install(DIRECTORY batteries
#		 DESTINATION ${BATT_INSTALL_INCLUDEDIR}
#		 FILES_MATCHING
#			 PATTERN "*.inc"
#			 PATTERN "*.h"
#	 )
# endif()	# BATT_ENABLE_INSTALL
