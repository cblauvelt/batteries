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

find_program(CLANG_FORMAT_BIN NAMES clang-format)

if(CLANG_FORMAT_BIN)
  message(STATUS "Found: clang-format")
  file(GLOB_RECURSE CPP_SOURCE_FILES *.cpp)
  file(GLOB_RECURSE CPP_HEADER_FILES *.h)

  add_custom_target(
    format-sources
    COMMAND clang-format --style=file -i ${CPP_SOURCE_FILES}
    COMMAND_EXPAND_LISTS VERBATIM)

  add_custom_target(
    format-headers
    COMMAND clang-format --style=file -i ${CPP_HEADER_FILES}
    COMMAND_EXPAND_LISTS VERBATIM)

  add_custom_target(
    format
    COMMENT "Running clang-format..."
    DEPENDS format-sources format-headers)
endif()
