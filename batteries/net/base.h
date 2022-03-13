// Copyright 2019 The Batteries Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <absl/strings/string_view.h>

#include "batteries/errors/error.h"

namespace batteries {

namespace net {

enum class url_error_code {
    no_error = 0,
    parse_error,
    escape_error,
    invalid_host_error,
    range_error,
};

// Define types
using query_value = std::pair<std::string, std::string>;
using query_values = std::vector<query_value>;
using error = errors::error;
using byte = unsigned char;

} // namespace net

} // namespace batteries

namespace std {
// Tell the C++ STL metaprogramming that enum ModbusErrorCode
// is registered with the standard error code system
template <>
struct is_error_code_enum<batteries::net::url_error_code> : true_type {};
} // namespace std

namespace batteries {

namespace net {

std::error_code make_error_code(batteries::net::url_error_code);

}

} // namespace batteries