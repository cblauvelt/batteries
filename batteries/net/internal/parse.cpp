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

#include <string>
#include <string_view>
#include <tuple>

#include <absl/strings/ascii.h>
#include <absl/strings/match.h>
#include <absl/strings/str_split.h>

#include "parse.hpp"

namespace batteries {

namespace net {

namespace internal {

std::tuple<std::string_view, std::string_view>
split(std::string_view s, std::string_view match, bool cutMatch) {
    size_t i = s.find(match, 0);
    if (i == s.npos) {
        return std::make_tuple(s, std::string_view());
    }
    if (cutMatch) {
        return std::make_tuple(s.substr(0, i), s.substr(i + match.length()));
    }
    return std::make_tuple(s.substr(0, i), s.substr(i));
}

bool valid_optional_port(std::string_view port) {
    // No port is a valid port
    if (port.empty()) {
        return true;
    }

    if (port.at(0) != ':') {
        return false;
    }
    for (auto c : port.substr(1)) {
        if (!absl::ascii_isdigit(c)) {
            return false;
        }
    }

    return true;
}

bool valid_userinfo(std::string_view s) {
    for (auto c : s) {
        if (!absl::ascii_isalnum(c) && c != '-' && c != '.' && c != '_' &&
            c != ':' && c != '~' && c != '!' && c != '$' && c != '&' &&
            c != '\'' && c != '(' && c != ')' && c != '*' && c != '+' &&
            c != ',' && c != ';' && c != '=' && c != '%' && c != '@') {
            return false;
        }
    }

    return true;
}

std::tuple<std::string, std::string_view, error>
parse_fragment(std::string_view rawurl) {
    std::string fragment;
    std::string_view fragment_view, rest;
    error err;

    std::tie(rest, fragment_view) = split(rawurl, "#", true);
    if (fragment_view.empty()) {
        return std::make_tuple(fragment, rest, err);
    }

    std::tie(fragment, err) = unescape(fragment_view, encoding::encodeFragment);
    if (err != errors::no_error) {
        return std::make_tuple(std::string(), rest, err);
    }

    return std::make_tuple(fragment, rest, err);
}

std::tuple<std::string, std::string_view, error>
parse_scheme(std::string_view rawurl) {
    for (int i = 0; i < rawurl.length(); i++) {
        byte c = rawurl[i];
        if (absl::ascii_isalpha(c)) {
            continue;
        } // do nothing

        if (absl::ascii_isdigit(c) || c == '+' || c == '-' || c == '.') {
            if (i == 0) {
                return std::make_tuple("", rawurl, errors::no_error);
            }
        }
        if (c == ':') {
            if (i == 0) {
                return std::make_tuple("", std::string_view(),
                                       error(url_error_code::parse_error,
                                             "missing protocol scheme"));
            }
            return std::make_tuple((std::string)rawurl.substr(0, i),
                                   rawurl.substr(i + 1), errors::no_error);
        }

        // we have encountered an invalid character,
        // so there is no valid scheme
        return std::make_tuple("", rawurl, errors::no_error);
    }
    return std::make_tuple("", rawurl, errors::no_error);
}

std::tuple<std::string, std::string, std::string_view, error>
parse_authority(std::string_view authority) {
    auto i = authority.rfind('@');

    if (i == authority.npos) {
        return std::make_tuple("", "", authority, errors::no_error);
    }

    std::string_view userinfo = authority.substr(0, i);

    if (!internal::valid_userinfo(userinfo)) {
        return std::make_tuple(
            "", "", std::string_view(),
            error(url_error_code::parse_error, "invalid userinfo"));
    }

    // Return values
    std::string username;
    std::string password;
    std::string_view host = authority.substr(i + 1);
    error err;

    // Has no password
    if (!absl::StrContains(userinfo, ":")) {
        std::tie(userinfo, err) =
            unescape(userinfo, internal::encoding::encodeUserPassword);
        if (err != errors::no_error) {
            return std::make_tuple("", "", host, err);
        }
        username = (std::string)userinfo;
    } else { // Has password
        std::string_view username_view;
        std::string_view password_view;
        std::tie(username_view, password_view) =
            internal::split(userinfo, ":", true);
        std::tie(username, err) =
            unescape(username_view, internal::encoding::encodeUserPassword);
        if (err != errors::no_error) {
            return std::make_tuple("", "", host, err);
        }
        std::tie(password, err) =
            unescape(password_view, internal::encoding::encodeUserPassword);
        if (err != errors::no_error) {
            return std::make_tuple("", "", host, err);
        }
    }
    return std::make_tuple(username, password, host, err);
}

std::tuple<std::string, std::string, error> parse_host(std::string_view host) {
    error err;
    std::string hostString;
    std::string portString;
    std::string_view port;

    // Handle IPv6
    if (absl::StartsWith(host, "[")) {
        // Parse an IP-Literal in RFC 3986 and RFC 6874.
        // E.g., "[fe80::1]", "[fe80::1%25en0]", "[fe80::1]:80".
        auto i = host.find("]");
        if (i == host.npos) {
            return std::make_tuple(
                "", "",
                error(url_error_code::parse_error, "missing ']' in host"));
        }
        port = host.substr(i + 1);
        if (!internal::valid_optional_port(port)) {
            return std::make_tuple(
                "", "",
                error(url_error_code::parse_error,
                      absl::StrCat("invalid port ", port, " after host")));
        }
        if (!port.empty()) {
            // Remove leading ':'
            portString = (std::string)port.substr(1);
        }

        // Remove port information and '[]' from the host
        host = host.substr(0, i + 1);

        // RFC 6874 defines that %25 (%-encoded percent) introduces
        // the zone identifier, and the zone identifier can use basically
        // any %-encoding it likes. That's different from the host, which
        // can only %-encode non-ASCII bytes.
        // We do impose some restrictions on the zone, to avoid stupidity
        // like newlines.
        auto zone = host.find("%25");
        if (zone != host.npos) {
            std::string host1;
            std::string host2;
            std::string host3;
            std::tie(host1, err) =
                unescape(host.substr(0, zone), internal::encoding::encodeHost);
            if (err != errors::no_error) {
                return std::make_tuple("", "", err);
            }
            std::tie(host2, err) =
                unescape(host.substr(zone, i), internal::encoding::encodeZone);
            if (err != errors::no_error) {
                return std::make_tuple("", "", err);
            }
            hostString = absl::StrCat(host1, host2);

            return std::make_tuple(hostString, portString, errors::no_error);
        }
    } else {

        // Handle IPv4
        auto i = host.rfind(':');
        if (i != host.npos) { // Process with port number
            port = (std::string)host.substr(i);
            if (!internal::valid_optional_port(port)) {
                return std::make_tuple(
                    "", "",
                    error(url_error_code::parse_error,
                          absl::StrCat("invalid port ", port, " after host")));
            }
            // Remove colon and assign to return value
            portString = (std::string)port.substr(1);
            // Remove port number from host string
            host = host.substr(0, i);
        }
    }

    std::tie(hostString, err) = unescape(host, internal::encoding::encodeHost);
    if (err != errors::no_error) {
        return std::make_tuple("", "", err);
    }

    return std::make_tuple(hostString, portString, errors::no_error);
}

std::tuple<query_map, error> parse_query(std::string_view query) {
    error err;
    query_map map;
    std::string key;
    std::string value;

    std::vector<std::string_view> splitResults =
        absl::StrSplit(query, absl::ByAnyChar("&;"));
    for (auto& result : splitResults) {
        if (result.empty()) {
            return std::make_tuple(query_map(),
                                   error(url_error_code::parse_error, query));
        }
        std::vector<std::string_view> keyValueResults =
            absl::StrSplit(result, '=');
        if (keyValueResults.size() != 2) {
            return std::make_tuple(query_map(),
                                   error(url_error_code::parse_error, query));
        }

        std::tie(key, err) =
            unescape(keyValueResults[0], encoding::encodeQueryComponent);
        if (err != errors::no_error) {
            break;
        }

        std::tie(value, err) =
            unescape(keyValueResults[1], encoding::encodeQueryComponent);
        if (err != errors::no_error) {
            break;
        }

        map.emplace(key, value);
    }

    return std::make_tuple(map, err);
}

} // namespace internal

} // namespace net

} // namespace batteries
