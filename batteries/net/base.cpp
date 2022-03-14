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

#include "base.hpp"

#include <absl/strings/str_cat.h>

namespace batteries {

namespace net {

namespace detail {

struct url_error_category : std::error_category {
    const char* name() const noexcept override { return "url_error_code"; }

    std::string message(int ev) const override {
        switch (static_cast<url_error_code>(ev)) {
        case url_error_code::no_error:
            return "Success";
        case url_error_code::parse_error:
            return "Parse Error";
        case url_error_code::escape_error:
            return "Escape Error";
        case url_error_code::invalid_host_error:
            return "Invalid host error";
        case url_error_code::range_error:
            return "The sequence '%' was not followed by two characters";
        default:
            return "(unrecognized error)";
        }
    }
};

const url_error_category theUrlErrorCategory{};

} // namespace detail

std::error_code make_error_code(url_error_code e) {
    return {static_cast<int>(e), detail::theUrlErrorCategory};
}

} // namespace net

} // namespace batteries
