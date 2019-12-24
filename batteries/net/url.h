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
#include <map>
#include <vector>
#include <utility>

#include <absl/strings/string_view.h>

#include "batteries/errors/error.h"

namespace batteries {

namespace net {

// Define types
enum class UrlErrorCode;
using QueryValue = std::pair<std::string, std::string>;
using QueryValues = std::vector<QueryValue>;
using UrlError = errors::Error<UrlErrorCode>;
using byte = unsigned char;

// Define constanst
const UrlError UrlNoError = UrlError();

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
 * @brief Takes a byte c that represents a value in hex and returns the decimal equivalent.
 * @param c The character to convert.
 */
byte unhex(byte c);

/**
 * @brief splits a string into two and only two parts at "match". If cutMatch is true, the delimiter is consumed.
 * @param s The string to split.
 * @param match The delimiter to search for and split if present.
 * @param cutMatch If true, the delimiter is consumed.
 */
std::tuple<absl::string_view, absl::string_view> split(absl::string_view s, absl::string_view match, bool cutMatch);

/**
 * @brief Determine if the port, if present, is a valid port number
 * @param port A string_view substring of the port portion of the URL.
 */
bool validOptionalPort(absl::string_view port);

/**
 * @brief reports whether s is a valid userinfo string per RFC 3986
 * 
 * Section 3.2.1:
 *     userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
 *     unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
 *     sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
 *                   / "*" / "+" / "," / ";" / "="
 *
 * It doesn't validate pct-encoded. The caller does that via func unescape.
 * 
 * @param s A string that reperesents the user info contained within a URL.
 * Usually of the form "username:password"
 */
bool validUserinfo(absl::string_view s);

/**
 * @brief unescapes a string; the mode specifies
 * which section of the URL string is being unescaped.
 * @param s A URL encoded string
 * @param mode The portion of the URL that is evaluated
 * @returns The decoded string and an error if any
 */
std::tuple<std::string, UrlError> unescape(absl::string_view s, encoding mode);

/**
 * @brief escapes a string; the mode specifies
 * which section of the URL string is being escaped.
 * @param s A raw string which contains reserved URL characters
 * @param mode The portion of the URL that is evaluated
 * @returns The encoded string
 */
std::string escape(absl::string_view s, encoding mode);

// Maybe rawurl is of the form scheme:path.
// (Scheme must be [a-zA-Z][a-zA-Z0-9+-.]*)
// If so, return scheme, path; else return "", rawurl.
std::tuple<absl::string_view, absl::string_view, UrlError>
parseScheme(absl::string_view rawurl);

bool shouldEscape(byte c, encoding mode);

} // end internal namespace

enum class UrlErrorCode {
    NoError = 0,
    ParseError,
    EscapeError,
    InvalidHostError,
    RangeError,
};

std::tuple<std::string, UrlError> unescapePath(absl::string_view path);
std::tuple<std::string, UrlError> unescapeQuery(absl::string_view query);
std::string escapePath(absl::string_view path);
std::string escapeQuery(absl::string_view query);

/**
 * A Url is a parsed URL/URI. The general form is "[scheme:][//[userinfo@]host][/]path[?query][#fragment]"
 */
class Url {
    
public:
    Url();

    // Parse functions
    UrlError parse(absl::string_view rawUrl);
    UrlError parseUri(absl::string_view rawUrl);
    UrlError setPath(absl::string_view path);

    // Query functions
    void add(QueryValue value);
    void add(std::string key, std::string value);
    void del(absl::string_view key);
    QueryValues get(absl::string_view key) const;
    void set(std::string key, std::string value);
    
    // Conveniance functions
    bool hasScheme() const;
    bool hasUsername() const;
    bool hasPassword() const;

    // Conversion functions
    std::string toString() const;
    std::string escapedPath() const;
    std::string escapedQuery() const;

public:
    // Errors
    static UrlError UrlParseError(absl::string_view s);
    static UrlError UrlEscapeError(absl::string_view s);
    static UrlError UrlInvalidHostError(absl::string_view s);
    static UrlError UrlRangeError(absl::string_view s);

private:
    UrlError parse(absl::string_view rawUrl, bool viaRequest);
    UrlError parseAuthority(absl::string_view authority);
    UrlError parseHost(absl::string_view host);

private:
    std::string mScheme;
    std::string mOpaque;
    std::string mUsername;
    std::string mPassword;
    std::string mHost;
    std::string mPath;
    std::string mRawPath;
    std::multimap<std::string,std::string> mQuery;
    std::string mRawQuery;
    bool mForceQuery;
    std::string mFragment;
};

std::string to_string(Url url);

}

}
