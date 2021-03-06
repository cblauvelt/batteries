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

#include "base.h"

#include <absl/strings/str_cat.h>

namespace batteries {

namespace net {

UrlError UrlParseError(absl::string_view s) {
    return UrlError(UrlErrorCode::ParseError, absl::StrCat("Parse Error: ", s));
}

UrlError UrlEscapeError(absl::string_view s) {
    return UrlError(UrlErrorCode::EscapeError, absl::StrCat("Escape Error: ", s));
}

UrlError UrlInvalidHostError(absl::string_view s) {
    return UrlError(UrlErrorCode::InvalidHostError, absl::StrCat("Invalid host error: ", s));
}

UrlError UrlRangeError(absl::string_view s) {
    return UrlError(UrlErrorCode::RangeError, absl::StrCat("'%' was not followed by two characters: ", s));
}

}

}
