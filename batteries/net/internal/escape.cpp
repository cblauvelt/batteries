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

#include <sstream>
#include <tuple>

#include <absl/strings/ascii.h>
#include <absl/strings/match.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>

#include "escape.hpp"

namespace batteries {

namespace net {

namespace internal {

byte unhex(byte c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    }
    if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    }
    if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    }

    return c;
}

bool shouldEscape(byte c, encoding mode) {
    // §2.3 Unreserved characters (alphanum)
    if (absl::ascii_isalnum(c)) {
        return false;
    }

    if (mode == internal::encoding::encodeHost ||
        mode == internal::encoding::encodeZone) {
        // §3.2.2 Host allows
        //	sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /
        //";" / "="
        // as part of reg-name.
        // We add : because we include :port as part of host.
        // We add [ ] because we include [ipv6]:port as part of host.
        // We add < > because they're the only characters left that
        // we could possibly allow, and Parse will reject them if we
        // escape them (because hosts can't use %-encoding for
        // ASCII bytes).
        if (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' ||
            c == ')' || c == '*' || c == '+' || c == ',' || c == ';' ||
            c == '=' || c == ':' || c == '[' || c == ']' || c == '<' ||
            c == '>' || c == '"') {
            return false;
        }
    }

    // §2.3 Unreserved characters (mark)
    if (c == '-' || c == '_' || c == '.' || c == '~') {
        return false;
    }

    // §2.2 Reserved characters (reserved)
    if (c == '$' || c == '&' || c == '+' || c == ',' || c == '/' || c == ':' ||
        c == ';' || c == '=' || c == '?' || c == '@') {
        // Different sections of the URL allow a few of
        // the reserved characters to appear unescaped.
        switch (mode) {
        case internal::encoding::encodePath: // §3.3
            // The RFC allows : @ & = + $ but saves / ; , for assigning
            // meaning to individual path segments. This package
            // only manipulates the path as a whole, so we allow those
            // last three as well. That leaves only ? to escape.
            return c == '?';

        case internal::encoding::encodePathSegment: // §3.3
            // The RFC allows : @ & = + $ but saves / ; , for assigning
            // meaning to individual path segments.
            return c == '/' || c == ';' || c == ',' || c == '?';

        case internal::encoding::encodeUserPassword: // §3.2.1
            // The RFC allows ';', ':', '&', '=', '+', '$', and ',' in
            // userinfo, so we must escape only '@', '/', and '?'.
            // The parsing of userinfo treats ':' as special so we must escape
            // that too.
            return c == '@' || c == '/' || c == '?' || c == ':';

        case internal::encoding::encodeQueryComponent: // §3.4
            // The RFC reserves (so we must escape) everything.
            return true;

        case internal::encoding::encodeFragment: // §4.1
            // The RFC text is silent but the grammar allows
            // everything, so escape nothing.
            return false;
        }
    }

    if (mode == internal::encoding::encodeFragment) {
        // RFC 3986 §2.2 allows not escaping sub-delims. A subset of sub-delims
        // are included in reserved from RFC 2396 §2.2. The remaining sub-delims
        // do not need to be escaped. To minimize potential breakage, we apply
        // two restrictions: (1) we always escape sub-delims outside of the
        // fragment, and (2) we always escape single quote to avoid breaking
        // callers that had previously assumed that single quotes would be
        // escaped.
        if (c == '!' || c == '(' || c == ')' || c == '*') {
            return false;
        }
    }

    // Everything else must be escaped.
    return true;
}

std::tuple<std::string, error> unescape(std::string_view s,
                                        internal::encoding mode) {
    // Count %, check that they're well-formed.
    int n = 0;
    bool hasPlus = false;
    std::ostringstream retVal;

    try {
        for (int i = 0; i < s.length(); i++) {
            switch (byte c = s.at(i)) {
            case '%':
                n++;
                if (!absl::ascii_isxdigit(s.at(i + 1)) ||
                    !absl::ascii_isxdigit(s.at(i + 2))) {
                    s = s.substr(i, i + 3);
                    return std::make_tuple(
                        "", error(url_error_code::escape_error, s));
                }
                // Per https://tools.ietf.org/html/rfc3986#page-21
                // in the host component %-encoding can only be used
                // for non-ASCII bytes.
                // But https://tools.ietf.org/html/rfc6874#section-2
                // introduces %25 being allowed to escape a percent sign
                // in IPv6 scoped-address literals. Yay.
                if (mode == internal::encoding::encodeHost &&
                    internal::unhex(s.at(i + 1)) < 8 &&
                    s.substr(i, i + 3) != "%25") {
                    return std::make_tuple("",
                                           error(url_error_code::escape_error,
                                                 s.substr(i, i + 3)));
                }
                if (mode == internal::encoding::encodeZone) {
                    // RFC 6874 says basically "anything goes" for zone
                    // identifiers and that even non-ASCII can be redundantly
                    // escaped, but it seems prudent to restrict %-escaped bytes
                    // here to those that are valid host name bytes in their
                    // unescaped form. That is, you can use escaping in the zone
                    // identifier but not to introduce bytes you couldn't just
                    // write directly. But Windows puts spaces here! Yay.
                    char v = internal::unhex(s.at(i + 1)) << 4 |
                             internal::unhex(s.at(i + 2));
                    if (s.substr(i, i + 3) != "%25" && v != ' ' &&
                        internal::shouldEscape(
                            v, internal::encoding::encodeHost)) {
                        return std::make_tuple(
                            "", error(url_error_code::escape_error,
                                      s.substr(i, i + 3)));
                    }
                }
                i += 2;
                break;
            case '+':
                hasPlus = mode == internal::encoding::encodeQueryComponent;
                break;
            default:
                if ((mode == internal::encoding::encodeHost ||
                     mode == internal::encoding::encodeZone) &&
                    c < 0x80 && shouldEscape(c, mode)) {
                    return std::make_tuple(
                        "", error(url_error_code::invalid_host_error,
                                  s.substr(i, i + 1)));
                }
            }
        }
    } catch (const std::out_of_range& e) {
        return std::make_tuple("", error(url_error_code::range_error, s));
    }

    // There is nothing to escape
    if (n == 0 && !hasPlus) {
        return std::make_tuple((std::string)s, error());
    }

    try {
        int j = 0;
        for (int i = 0; i < s.length(); i++) {
            switch (byte c = s.at(i)) {
            case '%':
                retVal.put(internal::unhex(s.at(i + 1)) << 4 |
                           internal::unhex(s.at(i + 2)));
                i += 2;
                break;
            case '+':
                if (mode == internal::encoding::encodeQueryComponent) {
                    retVal.put(' ');
                } else {
                    retVal.put('+');
                }
                break;
            default:
                retVal.put(c);
            }
        }
    } catch (const std::out_of_range& e) {
        return std::make_tuple("", error(url_error_code::range_error, s));
    }

    return std::make_tuple(retVal.str(), error());
}

std::string escape(std::string_view s, internal::encoding mode) {
    int spaceCount = 0;
    int hexCount = 0;

    for (auto c : s) {
        if (internal::shouldEscape(c, mode)) {
            if (c == ' ' && mode == internal::encoding::encodeQueryComponent) {
                spaceCount++;
            } else {
                hexCount++;
            }
        }
    }

    // Nothing to do
    if (spaceCount == 0 && hexCount == 0) {
        return (std::string)s;
    }

    auto required = s.length() + 2 * hexCount;

    // If no hex values were found, we only have to escape the spaces
    if (hexCount == 0) {
        std::string retVal = (std::string)s;
        for (int i = 0; i < s.length(); i++) {
            if (s[i] == ' ') {
                retVal[i] = '+';
            }
        }
        return retVal;
    }

    std::ostringstream escapedString;
    for (int i = 0; i < s.length(); i++) {
        byte c = s[i];
        if (c == ' ' && mode == internal::encoding::encodeQueryComponent) {
            escapedString.put('+');
        } else if (internal::shouldEscape(c, mode)) {
            escapedString.put('%');
            escapedString.put("0123456789ABCDEF"[c >> 4]); // high nibble
            escapedString.put("0123456789ABCDEF"[c & 15]); // low nibble
        } else {
            escapedString.put(s[i]);
        }
    }
    return escapedString.str();
}

} // namespace internal

} // namespace net

} // namespace batteries
