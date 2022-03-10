// Copyright 2019 The Batteries Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//	  https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <map>
#include <string>
#include <vector>

#include <absl/strings/string_view.h>

#include "base.h"
#include "batteries/errors/error.h"
#include "internal/escape.h"
#include "internal/parse.h"
#include "query.h"

namespace batteries {

namespace net {

class Url;

// Free Functions
std::tuple<std::string, UrlError> unescapePath(std::string_view path);
std::tuple<std::string, UrlError> unescapeQuery(std::string_view query);
std::string escapePath(std::string_view path);
std::string escapeQuery(std::string_view query);
Url ResolveReference(Url url);

/**
 * A Url is a parsed URL/URI. The general form is
 * "[scheme:][//[userinfo@]host][/]path[?query][#fragment]"
 */
class Url {

  public:
    /**
     * @brief Initializes a blank Url.
     */
    Url();

    /**
     * @brief Initializes a Url and parses the string rawurl using method
     * Url::parse.
     */
    Url(std::string rawurl);

    // Parse functions

    /**
     * @brief parse parses a URL from a string. All forms of relative URLs are
     * allowed.
     */
    UrlError parse(std::string_view rawUrl);

    /**
     * @brief parseUri parses a URL from a string. The URL is assumed to have
     * arrived via an HTTP request, in which case only absolute URLs or
     * path-absolute relative URLs are allowed.
     */
    UrlError parseUri(std::string_view rawUrl);

    // Setters/Getters
    /**
     * @brief scheme returns the scheme parsed from the url.
     * Example: For https://foo.com the scheme is 'https'.
     * @returns The scheme of the url.
     */
    std::string scheme() const;

    /**
     * @brief setScheme takes a string and sets the scheme of the url.
     * @param scheme A string that represents the scheme of the url.
     */
    void setScheme(std::string scheme);

    std::string opaque() const;
    void setOpaque(std::string opaque);

    /**
     * @brief username returns the username of the url.
     * @returns The username of the url.
     */
    std::string username() const;

    /**
     * @brief setUsername sets the username portion of the user info in the url.
     * Example: https://user@foo.com the username is 'user'.
     * @param username The name of the user.
     */
    void setUsername(std::string username);

    /**
     * @brief password returns the password of the url.
     * Example: https://user:pass@foo.com the password is 'pass'.
     * @returns The plaintext password of the password portion of the user info.
     */
    std::string password() const;

    /**
     * @brief setPassword sets the password portion of the user info in the url.
     * Example: https://user:pass@foo.com the password is 'pass'.
     * @param password The password of the user.
     */
    void setPassword(std::string password);

    /**
     * @brief host returns the host information.
     * @returns the host information in 'hostname:port' format.
     */
    std::string host() const;

    /**
     * @brief setHost takes a host in hostname[:port] format parses the hostname
     * and the port number. This different from setHost and setPort in that
     * there is some error checking during the parsing.
     * @param host The host in hostname[:port] format.
     * @returns A UrlError if any while parsing the input.
     */
    UrlError setHost(std::string host);

    /**
     * @brief hostname returns the host information.
     * @returns the hostname information.
     */
    std::string hostname() const;

    /**
     * @brief setHostname sets the hostname without any parsing to ensure that
     * it is valid.
     * @param hostname A string that represents the hostname.
     */
    void setHostname(std::string hostname);

    /**
     * @brief port returns the port information.
     * @returns the port information.
     */
    std::string port() const;

    /**
     * @brief setPort sets the port without any parsing to ensure that it is
     * valid.
     * @param port A string that represents the hostname.
     */
    void setPort(uint16_t port);

    /**
     * @brief path returns the path information.
     * @returns the path information.
     */
    std::string path() const;

    /**
     * @brief rawPath returns the unescaped path.
     * @returns the unescaped path information.
     */
    std::string rawPath() const;

    /**
     * @brief setPath sets the Path and RawPath fields of the URL based on the
     * provided escaped path p. It maintains the invariant that RawPath is only
     * specified when it differs from the default encoding of the path. For
     * example:
     * - setPath("/foo/bar")   will set Path="/foo/bar" and RawPath=""
     * - setPath("/foo%2fbar") will set Path="/foo/bar" and RawPath="/foo%2fbar"
     *
     * @param path The path to parse.
     * @returns setPath will return an error only if the provided path contains
     * an invalid escaping.
     */
    UrlError setPath(std::string_view path);

    /**
     * @brief query returns the query information.
     * @returns the query information.
     */
    Query query() const;

    /**
     * @brief setQuery sets the query.
     * @param query A list of key value pairs that represents the query.
     */
    void setQuery(const Query& query);

    /**
     * @brief fragment returns the fragment information.
     * @returns the fragment information.
     */
    std::string fragment() const;

    /**
     * @brief setFragment sets the fragment.
     * @param fragment A list of key value pairs that represents the query.
     * @returns UrlError Any error found while parsing the fragment.
     */
    UrlError setFragment(std::string fragment);

    // Conveniance functions
    bool hasScheme() const;
    bool hasUsername() const;
    bool hasPassword() const;

    // Conversion functions

    /**
     * @brief String reassembles the URL into a valid URL string.
     * The general form of the result is one of:
     *
     *	scheme:opaque?query#fragment
     *	scheme://userinfo@host/path?query#fragment
     *
     * If opaque() is non-empty, toString uses the first form;
     * otherwise it uses the second form.
     * Any non-ASCII characters in host are escaped.
     * To obtain the path, String uses escapedPath().
     *
     * In the second form, the following rules apply:
     *	- if Scheme is empty, scheme: is omitted.
     *	- if User is nil, userinfo@ is omitted.
     *	- if Host is empty, host/ is omitted.
     *	- if Scheme and u.Host are empty and u.User is nil,
     *	   the entire scheme://userinfo@host/ is omitted.
     *	- if Host is non-empty and u.Path begins with a /,
     *	   the form host/path does not add its own /.
     *	- if RawQuery is empty, ?query is omitted.
     *	- if Fragment is empty, #fragment is omitted.
     */
    std::string toString() const;
    std::string requestUri() const;
    std::string escapedPath() const;
    std::string escapedQuery() const;

    // Operators
    bool operator==(const Url& rhs) const;
    bool operator!=(const Url& rhs) const;

  private:
    UrlError parse(std::string_view rawUrl, bool viaRequest);

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

} // namespace net

} // namespace batteries

namespace std {

std::string to_string(batteries::net::Url url);

}