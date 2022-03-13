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

#include <sstream>
#include <string>
#include <string_view>

#include <absl/strings/str_cat.h>

#include "batteries/net/base.h"

#include "escape.h"

namespace batteries {

namespace net {

namespace internal {

using query_map = std::multimap<std::string, std::string>;

/**
 * @brief splits a string into two and only two parts at "match". If cutMatch is
 * true, the delimiter is consumed.
 * @param s The string to split.
 * @param match The delimiter to search for and split if present.
 * @param cutMatch If true, the delimiter is consumed.
 */
std::tuple<std::string_view, std::string_view>
split(std::string_view s, std::string_view match, bool cutMatch);

/**
 * @brief Determine if the port, if present, is a valid port number
 * @param port A string_view substring of the port portion of the URL.
 */
bool valid_optional_port(std::string_view port);

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
bool valid_userinfo(std::string_view s);

/**
 * @brief Takes a raw url that may contain the form path?query#fragment
 * and removes the fragment.
 * @param rawurl The url that may contain a fragment section.
 * @returns If there is a fragment found, returns fragment,path; else "",rawurl.
 */
std::tuple<std::string, std::string_view, error>
parse_fragment(std::string_view rawurl);

/**
 * @brief Takes a raw url that may contain the form scheme:path
 * and breaks it down into it's component parts.
 * @param rawurl The url that may contain a scheme section.
 * @returns If there is a scheme found, returns scheme,path; else "",rawurl.
 */
std::tuple<std::string, std::string_view, error>
parse_scheme(std::string_view rawurl);

/**
 * @brief parse_authority takes a string of form [userinfo@]host] and returns
 * The username and password, if any, and the string_view to pass on to
 * parse_host.
 *
 * @param authority a string_view of the form [userinfo@]host].
 * @returns The username, password, and the host portion of the input.
 */
std::tuple<std::string, std::string, std::string_view, error>
parse_authority(std::string_view authority);

/**
 * @brief parse_host parses the portion of the URL that contains the DNS or IP
 * address and, optionally, the port.
 * @param host The portion of the URL that contains the hostname information.
 * @returns A tuple containing the hostname, port number, and error if any.
 */
std::tuple<std::string, std::string, error> parse_host(std::string_view host);

/**
 * @brief Takes a raw query and converts it to a multimap of the values.
 * @param query The raw query to be parsed.
 * @returns A multimap[key] = []{value1, value2, ...}.
 * @returns error indicating an error while parsing if any.
 */
std::tuple<query_map, error> parse_query(std::string_view query);

/**
 * @brief Takes vector of key value pairs and build a query string.
 * @param begin A const_iterator to the begining of the values.
 * @param end A const_iterator to the end of the values.
 */
template <typename T> std::string build_query(T begin, T end) {
    std::ostringstream buf;
    for (auto it = begin; it != end; ++it) {
        auto key = escape(it->first, encoding::encodeQueryComponent);
        auto value = escape(it->second, encoding::encodeQueryComponent);
        buf << absl::StrCat(key, "=", value, "&");
    }

    std::string retVal = buf.str();
    // Remove the last &
    retVal.pop_back();
    return retVal;
}

} // namespace internal

} // namespace net

} // namespace batteries
