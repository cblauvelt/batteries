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

#include <string>
#include <vector>
#include <map>

#include <absl/strings/string_view.h>

#include "batteries/errors/error.h"

namespace batteries {

namespace net {

enum class UrlErrorCode {
    NoError = 0,
    ParseError,
    EscapeError,
    InvalidHostError,
    RangeError,
};

// Define types
using QueryValue = std::pair<std::string, std::string>;
using QueryValues = std::vector<QueryValue>;
using UrlError = errors::Error<UrlErrorCode>;
using byte = unsigned char;

// Errors
UrlError UrlParseError(absl::string_view s);
UrlError UrlEscapeError(absl::string_view s);
UrlError UrlInvalidHostError(absl::string_view s);
UrlError UrlRangeError(absl::string_view s);

// Define constanst
const UrlError UrlNoError = UrlError();

}

}