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

#include <ostream>
#include <string>

#include <absl/strings/str_cat.h>

namespace batteries {

namespace errors {

enum class generic_error_code { no_error = 0, generic_error };
}

} // namespace batteries

namespace std {
// Tell the C++ STL metaprogramming that enum generic_error_code
// is registered with the standard error code system
template <>
struct is_error_code_enum<batteries::errors::generic_error_code> : true_type {};
} // namespace std

namespace batteries {

namespace errors {

namespace detail {

struct generic_error_category : std::error_category {
    const char* name() const noexcept override { return "generic_error_code"; }

    std::string message(int ev) const override {
        switch (static_cast<generic_error_code>(ev)) {
        case generic_error_code::no_error:
            return "Success";
        case generic_error_code::generic_error:
            return "Generic error type";
        default:
            return "(unrecognized error)";
        }
    }
};

static const generic_error_category theGenericErrorCategory{};

} // namespace detail

inline std::error_code make_error_code(generic_error_code e) {
    return {static_cast<int>(e), detail::theGenericErrorCategory};
}

class error {

  public:
    error()
        : error_code_(generic_error_code::no_error)
        , message_() {}

    explicit error(std::error_code error_code)
        : error_code_(error_code)
        , message_(error_code.message()) {}

    explicit error(const std::string& error_message)
        : error_code_(generic_error_code::generic_error)
        , message_(error_message) {}

    explicit error(std::string_view error_message)
        : error_code_(generic_error_code::generic_error)
        , message_(error_message) {}

    explicit error(const char* error_message)
        : error_code_(generic_error_code::generic_error)
        , message_(error_message) {}

    explicit error(std::error_code error_code, const std::string& error_message)
        : error_code_(error_code)
        , message_(error_message) {}

    explicit error(std::error_code error_code, std::string_view error_message)
        : error_code_(error_code)
        , message_(error_message) {}

    explicit error(std::error_code error_code, const char* error_message)
        : error_code_(error_code)
        , message_(error_message) {}

    std::error_code error_code() const noexcept { return error_code_; }

    int value() const noexcept { return error_code_.value(); }

    const std::error_category& category() const noexcept {
        return error_code_.category();
    }

    std::string message() const {
        if (message_.empty()) {
            return error_code_.message();
        } else if (error_code_.message().empty()) {
            return message_;
        } else {
            return absl::StrCat(error_code_.message(), ": ", message_);
        }
    }

    std::string what() const { return message(); }

    std::runtime_error as_exception() const {
        return std::runtime_error(what());
    }

    bool operator==(const error& rhs) const {
        return (error_code_.value() ==
                rhs.error_code_.value()); // && message_ == rhs.message_);
    }

    bool operator==(const std::error_code& rhs) const {
        return (error_code_.value() == rhs.value());
    }

    bool operator!=(const error& rhs) const { return !(*this == rhs); }

    explicit operator bool() const { return (bool)error_code_; }

    bool operator!() const { return !((bool)*this); }

    friend std::ostream& operator<<(std::ostream& os, const error& error) {
        return os << "error_code: " << error.error_code_
                  << ", message: " << error.message();
    }

  private:
    std::error_code error_code_;
    std::string message_;
};

static const error no_error;

} // namespace errors

} // namespace batteries