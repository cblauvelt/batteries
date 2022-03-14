/*
Copyright (c) 2019 The Battery Authors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/**
 * This URL parsing was converted from golang.org/net/url
 */

#include "url.hpp"

#include <sstream>
#include <tuple>

#include <absl/strings/ascii.h>
#include <absl/strings/match.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>

#include "batteries/strings/match.h"

namespace batteries {

namespace net {

std::tuple<std::string, error> unescape_path(std::string_view path) {
    return internal::unescape(path, internal::encoding::encodePathSegment);
}

std::tuple<std::string, error> unescape_query(std::string_view query) {
    return internal::unescape(query, internal::encoding::encodeQueryComponent);
}

std::string escape_path(std::string_view path) {
    return internal::escape(path, internal::encoding::encodePathSegment);
}

std::string escape_query(std::string_view query) {
    return internal::escape(query, internal::encoding::encodeQueryComponent);
}

// TODO: Implement
url resolve_reference(url url) { return url; }

url::url()
    : scheme_()
    , opaque_()
    , username_()
    , password_()
    , host_()
    , port_()
    , path_()
    , raw_path_()
    , query_()
    , fragment_() {}

url::url(std::string rawurl)
    : scheme_()
    , opaque_()
    , username_()
    , password_()
    , host_()
    , port_()
    , path_()
    , raw_path_()
    , query_()
    , fragment_() {
    parse(rawurl);
}

error url::parse(std::string_view rawUrl) { return parse(rawUrl, false); }
error url::parse_uri(std::string_view rawUrl) { return parse(rawUrl, true); }

std::string url::scheme() const { return scheme_; }

void url::set_scheme(std::string scheme) { scheme_ = scheme; }

std::string url::opaque() const { return opaque_; }

void url::set_opaque(std::string opaque) { opaque_ = opaque; }

std::string url::username() const { return username_; }

void url::set_username(std::string username) { username_ = username; }

std::string url::password() const { return password_; }

void url::set_password(std::string password) { password_ = password; }

std::string url::host() const {
    if (!port_.empty()) {
        return absl::StrCat(host_, ":", port_);
    }

    return host_;
}

error url::set_host(std::string host) {
    error err;
    std::tie(host_, port_, err) = internal::parse_host(host);
    return err;
}

std::string url::hostname() const { return host_; }

void url::set_hostname(std::string hostname) { host_ = hostname; }

std::string url::port() const { return port_; }

void url::set_port(uint16_t port) { port_ = absl::StrCat(port); }

std::string url::path() const { return path_; }

std::string url::raw_path() const { return raw_path_; }

error url::set_path(std::string_view path) {
    error err;
    std::tie(path_, err) =
        internal::unescape(path, internal::encoding::encodePath);
    if (err != errors::no_error) {
        return err;
    }

    std::string escaped_path =
        internal::escape(path_, internal::encoding::encodePath);
    if (path_ == escaped_path) {
        // Default encoding is fine.
        raw_path_ = "";
    } else {
        raw_path_ = (std::string)path;
    }
    return errors::no_error;
}

net::query url::query() const { return query_; }

void url::set_query(const net::query& query) { query_ = query; }

std::string url::fragment() const {
    return internal::escape(fragment_, internal::encoding::encodeFragment);
}

error url::set_fragment(std::string fragment) {
    error err;
    std::tie(fragment_, err) =
        internal::unescape(fragment, internal::encoding::encodeFragment);

    return err;
}

// Conveniance functions
bool url::has_scheme() const { return !scheme_.empty(); }
bool url::has_username() const { return !username_.empty(); }
bool url::has_password() const { return !password_.empty(); }

std::string url::to_string() const {
    std::ostringstream buf;

    if (path_ == "*") {
        return "*";
    }
    if (!scheme_.empty()) {
        buf << scheme_ << ':';
    }
    if (!opaque_.empty()) {
        buf << opaque_;
    } else {
        if (!scheme_.empty() || !host_.empty() || !username_.empty()) {
            if (!host_.empty() || !path_.empty() || !username_.empty()) {
                buf << "//";
            }
            if (!username_.empty()) {
                buf << username_;
                if (!password_.empty()) {
                    buf << ":" << password_;
                }
                buf << '@';
            }
            if (!host_.empty()) {
                buf << internal::escape(host_, internal::encoding::encodeHost);
                if (!port_.empty()) {
                    buf << ":" << port_;
                }
            }
        }
        auto path = escaped_path();
        if (!path.empty() && path[0] != '/' && !host_.empty()) {
            buf << '/';
        }
        if (buf.tellp() == 0) {
            // RFC 3986 §4.2
            // A path segment that contains a colon character (e.g.,
            // "this:that") cannot be used as the first segment of a
            // relative-path reference, as it would be mistaken for a scheme
            // name. Such a segment must be preceded by a dot-segment (e.g.,
            // "./this:that") to make a relative- path reference.
            auto i = path.find(':');
            if (i != path.npos && path.substr(0, i).find('/') != path.npos) {
                buf << "./";
            }
        }
        buf << path;
    }

    buf << query_.to_string();

    if (!fragment_.empty()) {
        buf << '#'
            << internal::escape(fragment_, internal::encoding::encodeFragment);
    }
    return buf.str();
}

std::string url::request_uri() const {
    auto result = opaque_;
    if (result == "") {
        result = escaped_path();
        if (result == "") {
            result = "/";
        }
    } else {
        if (absl::StartsWith(result, "//")) {
            result = scheme_ + ":" + result;
        }
    }
    absl::StrAppend(&result, query_.to_string());

    return result;
}

std::string url::escaped_path() const {
    return internal::escape(path_, internal::encoding::encodePath);
}

std::string url::escaped_query() const {
    std::string query;
    return internal::escape(query, internal::encoding::encodeQueryComponent);
}

bool url::operator==(const url& rhs) const {
    return (scheme_ == rhs.scheme_ && opaque_ == rhs.opaque_ &&
            username_ == rhs.username_ && password_ == rhs.password_ &&
            host_ == rhs.host_ && port_ == rhs.port_ && path_ == rhs.path_ &&
            raw_path_ == rhs.raw_path_ && query_ == rhs.query_ &&
            fragment_ == rhs.fragment_);
}

bool url::operator!=(const url& rhs) const { return !(*this == rhs); }

error url::parse(std::string_view rawurl, bool viaRequest) {
    std::string_view rest;
    error err;

    if (strings::contains_ctl_char(rawurl)) {
        return error(url_error_code::parse_error,
                     "invalid control character in URL");
    }

    if (rawurl.empty() && viaRequest) {
        return error(url_error_code::parse_error, "empty url");
    }

    if (rawurl == "*") {
        path_ = "*";
        return errors::no_error;
    }

    // Split off fragment
    std::tie(fragment_, rest, err) = internal::parse_fragment(rawurl);

    // Split off possible leading "http:", "mailto:", etc.
    // Cannot contain escaped characters.
    std::string_view scheme;
    std::tie(scheme, rest, err) = internal::parse_scheme(rest);
    if (err != errors::no_error) {
        return err;
    }
    scheme_ = absl::AsciiStrToLower(scheme);

    if (absl::EndsWith(rest, "?") && strings::count(rest, "?") == 1) {
        query_.set_force_query(true);
        rest = rest.substr(0, rest.length() - 1);
    } else {
        std::string_view rawQuery;
        std::tie(rest, rawQuery) = internal::split(rest, "?", true);
        query_.parse((std::string)rawQuery);
    }

    if (!absl::StartsWith(rest, "/")) {
        if (scheme_.empty()) {
            // We consider rootless paths per RFC 3986 as opaque.
            opaque_ = (std::string)rest;
            return errors::no_error;
        }
        if (viaRequest) {
            return error(url_error_code::parse_error,
                         "invalid URI for request");
        }

        // Avoid confusion with malformed schemes, like cache_object:foo/bar.
        // See golang.org/issue/16822.
        //
        // RFC 3986, §3.3:
        // In addition, a URI reference (Section 4.1) may be a relative-path
        // reference, in which case the first path segment cannot contain a
        // colon (":") character.
        auto colon = rest.find(':');
        auto slash = rest.find('/');
        if (colon >= 0 && (slash < 0 || colon < slash)) {
            // First path segment has colon. Not allowed in relative URL.
            return error(url_error_code::parse_error,
                         "first path segment in URL cannot contain colon");
        }
    }

    if (!scheme_.empty() || !viaRequest && !absl::StartsWith(rest, "///") &&
                                absl::StartsWith(rest, "//")) {
        std::string_view authority;
        std::string_view host;

        // Separate authority@host from the path
        std::tie(authority, rest) = internal::split(rest.substr(2), "/", false);

        // Parse the username and password
        std::tie(username_, password_, host, err) =
            internal::parse_authority(authority);
        if (err != errors::no_error) {
            return err;
        }

        // Parse the host
        std::tie(host_, port_, err) = internal::parse_host(host);
        if (err != errors::no_error) {
            return err;
        }
    }
    // Set Path and, optionally, RawPath.
    // RawPath is a hint of the encoding of Path. We don't want to set it if
    // the default escaping of Path is equivalent, to help make sure that people
    // don't rely on it in general.
    err = set_path(rest);
    if (err != errors::no_error) {
        return err;
    }
    return errors::no_error;
}

} // namespace net

} // namespace batteries

namespace std {

std::string to_string(batteries::net::url url) { return url.to_string(); }

} // namespace std