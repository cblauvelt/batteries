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
#include "batteries/errors/error.hpp"
#include "internal/escape.h"
#include "internal/parse.h"
#include "query.h"

namespace batteries {

namespace net {

class url;

// Free Functions
std::tuple<std::string, error> unescape_path(std::string_view path);
std::tuple<std::string, error> unescape_query(std::string_view query);
std::string escape_path(std::string_view path);
std::string escape_query(std::string_view query);
url resolve_reference(url url);

/**
 * A url is a parsed URL/URI. The general form is
 * "[scheme:][//[userinfo@]host][/]path[?query][#fragment]"
 */
class url {

  public:
    /**
     * @brief Initializes a blank url.
     */
    url();

    /**
     * @brief Initializes a url and parses the string rawurl using method
     * url::parse.
     */
    url(std::string rawurl);

    // Parse functions

    /**
     * @brief parse parses a URL from a string. All forms of relative URLs are
     * allowed.
     */
    error parse(std::string_view rawUrl);

    /**
     * @brief parse_uri parses a URL from a string. The URL is assumed to have
     * arrived via an HTTP request, in which case only absolute URLs or
     * path-absolute relative URLs are allowed.
     */
    error parse_uri(std::string_view rawUrl);

    // Setters/Getters
    /**
     * @brief scheme returns the scheme parsed from the url.
     * Example: For https://foo.com the scheme is 'https'.
     * @returns The scheme of the url.
     */
    std::string scheme() const;

    /**
     * @brief set_scheme takes a string and sets the scheme of the url.
     * @param scheme A string that represents the scheme of the url.
     */
    void set_scheme(std::string scheme);

    std::string opaque() const;
    void set_opaque(std::string opaque);

    /**
     * @brief username returns the username of the url.
     * @returns The username of the url.
     */
    std::string username() const;

    /**
     * @brief set_username sets the username portion of the user info in the
     * url. Example: https://user@foo.com the username is 'user'.
     * @param username The name of the user.
     */
    void set_username(std::string username);

    /**
     * @brief password returns the password of the url.
     * Example: https://user:pass@foo.com the password is 'pass'.
     * @returns The plaintext password of the password portion of the user info.
     */
    std::string password() const;

    /**
     * @brief set_password sets the password portion of the user info in the
     * url. Example: https://user:pass@foo.com the password is 'pass'.
     * @param password The password of the user.
     */
    void set_password(std::string password);

    /**
     * @brief host returns the host information.
     * @returns the host information in 'hostname:port' format.
     */
    std::string host() const;

    /**
     * @brief set_host takes a host in hostname[:port] format parses the
     * hostname and the port number. This different from set_host and set_port
     * in that there is some error checking during the parsing.
     * @param host The host in hostname[:port] format.
     * @returns A error if any while parsing the input.
     */
    error set_host(std::string host);

    /**
     * @brief hostname returns the host information.
     * @returns the hostname information.
     */
    std::string hostname() const;

    /**
     * @brief set_hostname sets the hostname without any parsing to ensure that
     * it is valid.
     * @param hostname A string that represents the hostname.
     */
    void set_hostname(std::string hostname);

    /**
     * @brief port returns the port information.
     * @returns the port information.
     */
    std::string port() const;

    /**
     * @brief set_port sets the port without any parsing to ensure that it is
     * valid.
     * @param port A string that represents the hostname.
     */
    void set_port(uint16_t port);

    /**
     * @brief path returns the path information.
     * @returns the path information.
     */
    std::string path() const;

    /**
     * @brief raw_path returns the unescaped path.
     * @returns the unescaped path information.
     */
    std::string raw_path() const;

    /**
     * @brief set_path sets the Path and RawPath fields of the URL based on the
     * provided escaped path p. It maintains the invariant that RawPath is only
     * specified when it differs from the default encoding of the path. For
     * example:
     * - set_path("/foo/bar")   will set Path="/foo/bar" and RawPath=""
     * - set_path("/foo%2fbar") will set Path="/foo/bar" and
     * RawPath="/foo%2fbar"
     *
     * @param path The path to parse.
     * @returns set_path will return an error only if the provided path contains
     * an invalid escaping.
     */
    error set_path(std::string_view path);

    /**
     * @brief query returns the query information.
     * @returns the query information.
     */
    net::query query() const;

    /**
     * @brief setquery sets the query.
     * @param query A list of key value pairs that represents the query.
     */
    void set_query(const net::query& query);

    /**
     * @brief fragment returns the fragment information.
     * @returns the fragment information.
     */
    std::string fragment() const;

    /**
     * @brief set_fragment sets the fragment.
     * @param fragment A list of key value pairs that represents the query.
     * @returns error Any error found while parsing the fragment.
     */
    error set_fragment(std::string fragment);

    // Conveniance functions
    bool has_scheme() const;
    bool has_username() const;
    bool has_password() const;

    // Conversion functions

    /**
     * @brief String reassembles the URL into a valid URL string.
     * The general form of the result is one of:
     *
     *	scheme:opaque?query#fragment
     *	scheme://userinfo@host/path?query#fragment
     *
     * If opaque() is non-empty, to_string uses the first form;
     * otherwise it uses the second form.
     * Any non-ASCII characters in host are escaped.
     * To obtain the path, String uses escaped_path().
     *
     * In the second form, the following rules apply:
     *	- if Scheme is empty, scheme: is omitted.
     *	- if User is nil, userinfo@ is omitted.
     *	- if Host is empty, host/ is omitted.
     *	- if Scheme and u.Host are empty and u.User is nil,
     *	   the entire scheme://userinfo@host/ is omitted.
     *	- if Host is non-empty and u.Path begins with a /,
     *	   the form host/path does not add its own /.
     *	- if Rawquery is empty, ?query is omitted.
     *	- if Fragment is empty, #fragment is omitted.
     */
    std::string to_string() const;
    std::string request_uri() const;
    std::string escaped_path() const;
    std::string escaped_query() const;

    // Operators
    bool operator==(const url& rhs) const;
    bool operator!=(const url& rhs) const;

  private:
    error parse(std::string_view rawUrl, bool viaRequest);

  private:
    std::string scheme_;
    std::string opaque_;
    std::string username_;
    std::string password_;
    std::string host_;
    std::string port_;
    std::string path_;
    std::string raw_path_;
    net::query query_;
    std::string fragment_;
};

} // namespace net

} // namespace batteries

namespace std {

std::string to_string(batteries::net::url url);

}