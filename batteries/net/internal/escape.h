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

#include <string_view>

#include "batteries/net/base.h"

namespace batteries {

namespace net {

namespace internal {

// This determines what part of the encoding/decoding is being completed
enum class encoding : uint8_t {
    encodePath,
    encodePathSegment,
    encodeHost,
    encodeZone,
    encodeUserPassword,
    encodeQueryComponent,
    encodeFragment,
};

/**
 * @brief Takes a byte c that represents a value in hex and returns the decimal
 * equivalent.
 * @param c The character to convert.
 */
byte unhex(byte c);

/**
 * @brief Determines whether a character should be escaped based on the
 * charachter and the current encoding mode being used, i.e. query segment,
 * host segment, etc.
 *
 * @param c The character that is being examined.
 * @param mode The current encoding mode.
 * @returns Whether or not the character should be escaped to the form %xx.
 */
bool shouldEscape(byte c, encoding mode);

/**
 * @brief unescapes a string; the mode specifies
 * which section of the URL string is being unescaped.
 * @param s A URL encoded string
 * @param mode The portion of the URL that is evaluated
 * @returns The decoded string and an error if any
 */
std::tuple<std::string, UrlError> unescape(std::string_view s, encoding mode);

/**
 * @brief escapes a string; the mode specifies
 * which section of the URL string is being escaped.
 * @param s A raw string which contains reserved URL characters
 * @param mode The portion of the URL that is evaluated
 * @returns The encoded string
 */
std::string escape(std::string_view s, encoding mode);

} // namespace internal

} // namespace net

} // namespace batteries
