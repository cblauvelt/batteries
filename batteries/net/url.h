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
#include "base.h"
#include "query.h"
#include "internal/parse.h"
#include "internal/escape.h"

namespace batteries {

namespace net {

// Free Functions
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

    // Setters/Getters
    std::string scheme() const;
    void setScheme(std::string scheme);

    std::string opaque() const;
    void setOpaque(std::string opaque);

    std::string username() const;
    void setUsername(std::string username);

    std::string password() const;
    void setPassword(std::string password);

    std::string host() const;
    UrlError setHost(std::string host);

    std::string port() const;
    void setPort(uint16_t port);

    std::string path() const;
    std::string rawPath() const;
    /** setPath sets the Path and RawPath fields of the URL based on the provided
     * escaped path p. It maintains the invariant that RawPath is only specified
     * when it differs from the default encoding of the path.
     * For example:
     * - setPath("/foo/bar")   will set Path="/foo/bar" and RawPath=""
     * - setPath("/foo%2fbar") will set Path="/foo/bar" and RawPath="/foo%2fbar"
     * setPath will return an error only if the provided path contains an invalid
     * escaping.
     */
    UrlError setPath(absl::string_view path);

    Query query() const;
    void setQuery(const Query& query);

    std::string fragment() const;
    void setFragment(std::string fragment);

    
    // Conveniance functions
    bool hasScheme() const;
    bool hasUsername() const;
    bool hasPassword() const;

    // Conversion functions
    std::string toString() const;
    std::string escapedPath() const;
    std::string escapedQuery() const;

private:
    UrlError parse(absl::string_view rawUrl, bool viaRequest);
    UrlError parseAuthority(absl::string_view authority);

private:
    std::string mScheme;
    std::string mOpaque;
    std::string mUsername;
    std::string mPassword;
    std::string mHost;
    std::string mPort;
    std::string mPath;
    std::string mRawPath;
    Query mQuery;
    std::string mFragment;
};

}

}

namespace std {

std::string to_string(batteries::net::Url url);

}