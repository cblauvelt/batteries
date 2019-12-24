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

#include "url.h"

#include <tuple>
#include <sstream>

#include <absl/strings/ascii.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>
#include <absl/strings/match.h>

#include "batteries/strings/match.h"

namespace batteries {

namespace net {

namespace internal {

byte unhex(byte c) {
    if('0' <= c && c <= '9')
		return c - '0';
	if('a' <= c && c <= 'f')
		return c - 'a' + 10;
	if('A' <= c && c <= 'F')
		return c - 'A' + 10;
}

std::tuple<absl::string_view, absl::string_view>
split(absl::string_view s, absl::string_view match, bool cutMatch) {
	size_t i = s.find(match, 0);
	if(i == s.npos) {
		return std::make_tuple(s, absl::string_view());
	}
	if(cutMatch) {
		return std::make_tuple(
			s.substr(0,i),
			s.substr(i+match.length())
		);
	}
	return std::make_tuple(
		s.substr(0,i),
		s.substr(i)
	);
}

std::tuple<std::string, UrlError> unescape(absl::string_view s, internal::encoding mode) {
	// Count %, check that they're well-formed.
	int n = 0;
	bool hasPlus = false;
    std::ostringstream retVal;

    try {
        for(int i = 0; i < s.length(); i++) {
            switch (byte c = s.at(i)) {
            case '%':
                n++;
                if(!absl::ascii_isxdigit(s.at(i+1)) || !absl::ascii_isxdigit(s.at(i+2))) {
                    s = s.substr(i, i+3);
                    return std::make_tuple("", Url::UrlEscapeError(s));
                }
                // Per https://tools.ietf.org/html/rfc3986#page-21
                // in the host component %-encoding can only be used
                // for non-ASCII bytes.
                // But https://tools.ietf.org/html/rfc6874#section-2
                // introduces %25 being allowed to escape a percent sign
                // in IPv6 scoped-address literals. Yay.
                if(mode == internal::encoding::encodeHost && internal::unhex(s.at(i+1)) < 8 && s.substr(i,i+3) != "%25") {
                    return std::make_tuple(
                        "",
                        Url::UrlEscapeError(s.substr(i,i+3))
                    );
                }
                if(mode == internal::encoding::encodeZone) {
                    // RFC 6874 says basically "anything goes" for zone identifiers
                    // and that even non-ASCII can be redundantly escaped,
                    // but it seems prudent to restrict %-escaped bytes here to those
                    // that are valid host name bytes in their unescaped form.
                    // That is, you can use escaping in the zone identifier but not
                    // to introduce bytes you couldn't just write directly.
                    // But Windows puts spaces here! Yay.
                    char v = internal::unhex(s.at(i+1))<<4 | internal::unhex(s.at(i+2));
                    if(
						s.substr(i,i+3) != "%25" &&
						v != ' ' &&
						internal::shouldEscape(v, internal::encoding::encodeHost)
					) {
                        return std::make_tuple(
                            "",
                            Url::UrlEscapeError(s.substr(i,i+3))
                        );
                    }
                }
                i += 2;
				break;
            case '+':
                hasPlus = mode == internal::encoding::encodeQueryComponent;
				break;
            default:
                if((mode == internal::encoding::encodeHost || mode == internal::encoding::encodeZone) &&
                    c < 0x80 && shouldEscape(c, mode) ) {
                    return std::make_tuple(
                        "",
                        Url::UrlInvalidHostError(s.substr(i, i+1))
                    );
                }
            }
        }
    } catch (const std::out_of_range& e) {
        return std::make_tuple(
            "",
            Url::UrlRangeError(s)
        );
    }

    // There is nothing to escape
	if(n == 0 && !hasPlus) {
        return std::make_tuple((std::string)s, UrlError());
	}

    try {
		int j = 0;
        for(int i = 0; i < s.length(); i++) {
            switch (byte c = s.at(i)) {
            case '%':
                retVal.put(internal::unhex(s.at(i+1))<<4 | internal::unhex(s.at(i+2)));
                i += 2;
				break;
            case '+':
                if(mode == internal::encoding::encodeQueryComponent) {
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
        return std::make_tuple(
            "",
            Url::UrlRangeError(s)
        );
    }

	return std::make_tuple(retVal.str(), UrlError());
}

std::string escape(absl::string_view s, internal::encoding mode) {
	int spaceCount = 0;
    int hexCount = 0;

    for(auto c: s) {
		if(internal::shouldEscape(c, mode)) {
			if(c == ' ' && mode == internal::encoding::encodeQueryComponent) {
				spaceCount++;
			} else {
				hexCount++;
			}
		}
	}

    // Nothing to do
	if(spaceCount == 0 && hexCount == 0) {
		return (std::string)s;
	}
    
	auto required = s.length() + 2*hexCount;
    
    // If no hex values were found, we only have to escape the spaces
	if(hexCount == 0) {
		std::string retVal = (std::string)s;
		for(int i = 0; i < s.length(); i++) {
			if(s[i] == ' ') {
				retVal[i] = '+';
			}
		}
		return retVal;
	}

	std::ostringstream escapedString;
    for(int i = 0; i < s.length(); i++) {
        byte c = s[i];
		if(c == ' ' && mode == internal::encoding::encodeQueryComponent) {
			escapedString.put('+');
	    } else if(internal::shouldEscape(c, mode)) {
			escapedString.put('%');
			escapedString.put("0123456789ABCDEF"[c>>4]); // high nibble
			escapedString.put("0123456789ABCDEF"[c&15]); // low nibble
	    } else {
			escapedString.put(s[i]);
		}
	}
	return escapedString.str();
}

bool validOptionalPort(absl::string_view port) {
	// No port is a valid port
	if(port.empty()) { return true;}

	if(port.at(0) != ':') {
		return false;
	}

	for(auto c : port.substr(1)) {
		if(!absl::ascii_isdigit(c)) { return false;}
	}

	return true;
}

bool validUserinfo(absl::string_view s) {
	for(auto c : s) {
		if(!absl::ascii_isalnum(c) || c != '-' || c!= '.' || c!= '_' ||
			c!= ':' || c!= '~' || c!= '!' || c!= '$' || c!= '&' ||
			c!= '\'' || c!= '(' || c!= ')' || c!= '*' ||
			c!= '+' || c!= ',' || c!= ';' || c!= '=' || c!= '%' || c!= '@')
		{
			return false;
		}
	}

	return true;
}

std::tuple<absl::string_view, absl::string_view, UrlError> parseScheme(absl::string_view rawurl) {
	for(int i = 0; i < rawurl.length(); i++) {
		byte c = rawurl[i];
		if(absl::ascii_isalpha(c)) { continue;} // do nothing
		
        if(absl::ascii_isdigit(c) || c == '+' || c == '-' || c == '.') {
			if(i == 0) {
				return std::make_tuple(
                    absl::string_view(),
                    rawurl,
                    UrlNoError
                );
			}
        }
		if(c == ':') {
			if(i == 0) {
				return std::make_tuple(
                    absl::string_view(),
                    absl::string_view(),
                    Url::UrlParseError("missing protocol scheme")
                );
			}
            return std::make_tuple(
                rawurl.substr(0,i),
                rawurl.substr(i+1),
                UrlNoError
            );
        }
		
        // we have encountered an invalid character,
        // so there is no valid scheme
        return std::make_tuple(
            absl::string_view(), rawurl, UrlNoError
        );
	}
    return std::make_tuple(
        absl::string_view(), rawurl, UrlNoError
    );
}

bool shouldEscape(byte c, encoding mode) {
    // §2.3 Unreserved characters (alphanum)
	if(absl::ascii_isalnum(c)) { return false; }

   if(mode == internal::encoding::encodeHost || mode == internal::encoding::encodeZone) {
		// §3.2.2 Host allows
		//	sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
		// as part of reg-name.
		// We add : because we include :port as part of host.
		// We add [ ] because we include [ipv6]:port as part of host.
		// We add < > because they're the only characters left that
		// we could possibly allow, and Parse will reject them if we
		// escape them (because hosts can't use %-encoding for
		// ASCII bytes).
		if(c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' ||
            c == '*' || c == '+' || c == ',' || c == ';' || c == '=' || c == ':' ||
            c == '[' || c == ']' || c == '<' || c == '>' || c == '"')
        {
            return false;
        }
	}

    // §2.3 Unreserved characters (mark)
    if(c == '-' || c == '_' || c == '.' || c == '~') { return false; }
	
    // §2.2 Reserved characters (reserved)
    if(c == '$' || c == '&' || c == '+' || c == ',' || c == '/' || c == ':' ||
        c == ';' || c == '=' || c == '?' || c == '@')
    {
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

	if(mode == internal::encoding::encodeFragment) {
		// RFC 3986 §2.2 allows not escaping sub-delims. A subset of sub-delims are
		// included in reserved from RFC 2396 §2.2. The remaining sub-delims do not
		// need to be escaped. To minimize potential breakage, we apply two restrictions:
		// (1) we always escape sub-delims outside of the fragment, and (2) we always
		// escape single quote to avoid breaking callers that had previously assumed that
		// single quotes would be escaped.
        if(c == '!' || c == '(' || c == ')' || c == '*') { return false;}
	}

	// Everything else must be escaped.
    return true;
}

}

std::tuple<std::string, UrlError> unescapePath(absl::string_view path) {
	return internal::unescape(path, internal::encoding::encodePathSegment);
}

std::tuple<std::string, UrlError> unescapeQuery(absl::string_view query) {
	return internal::unescape(query, internal::encoding::encodeQueryComponent);
}

std::string escapePath(absl::string_view path) {
	return internal::escape(path, internal::encoding::encodePathSegment);
}

std::string escapeQuery(absl::string_view query) {
	return internal::escape(query, internal::encoding::encodeQueryComponent);
}

Url::Url() :
    mScheme(),
	mOpaque(),
    mUsername(),
    mPassword(),
    mHost(),
    mPath(),
    mRawPath(),
    mQuery(),
	mRawQuery(),
    mForceQuery(false),
    mFragment()
{}

UrlError Url::parse(absl::string_view rawUrl) {
	parse(rawUrl, true);
}
UrlError Url::parseUri(absl::string_view rawUrl) {
	parse(rawUrl, true);
}

/** setPath sets the Path and RawPath fields of the URL based on the provided
 * escaped path p. It maintains the invariant that RawPath is only specified
 * when it differs from the default encoding of the path.
 * For example:
 * - setPath("/foo/bar")   will set Path="/foo/bar" and RawPath=""
 * - setPath("/foo%2fbar") will set Path="/foo/bar" and RawPath="/foo%2fbar"
 * setPath will return an error only if the provided path contains an invalid
 * escaping.
 */
UrlError Url::setPath(absl::string_view path) {
	UrlError err;
	std::tie(path, err) = internal::unescape(path, internal::encoding::encodePath);
	if(err != UrlNoError) {
		return err;
	}
	mPath = (std::string)path;
	std::string escapedPath = internal::escape(path, internal::encoding::encodePath);
	if(path == escapedPath) {
		// Default encoding is fine.
		mRawPath = "";
	} else {
		mRawPath = (std::string)path;
	}
	return UrlNoError;
}

// Query functions
void Url::add(QueryValue value) {}
void Url::add(std::string key, std::string value) {}
void Url::del(absl::string_view key) {}
QueryValues Url::get(absl::string_view key) const {}
void Url::set(std::string key, std::string value) {}

// Conveniance functions
bool Url::hasScheme() const {
    return !mScheme.empty();
}
bool Url::hasUsername() const {
    return !mUsername.empty();
}
bool Url::hasPassword() const {
    return !mPassword.empty();
}

std::string Url::toString() const {
    std::string urlString;
}

std::string Url::escapedPath() const {
    return internal::escape(mPath, internal::encoding::encodePathSegment);
}

std::string Url::escapedQuery() const {
	std::string query;
    return internal::escape(query, internal::encoding::encodeQueryComponent);
}

UrlError Url::Url::UrlParseError(absl::string_view s) {
    return UrlError(UrlErrorCode::ParseError, absl::StrCat("Parse Error: ", s));
}

UrlError Url::UrlEscapeError(absl::string_view s) {
    return UrlError(UrlErrorCode::EscapeError, absl::StrCat("Escape Error: ", s));
}

UrlError Url::UrlInvalidHostError(absl::string_view s) {
    return UrlError(UrlErrorCode::InvalidHostError, absl::StrCat("Invalid host error: ", s));
}

UrlError Url::UrlRangeError(absl::string_view s) {
    return UrlError(UrlErrorCode::RangeError, absl::StrCat("'%' was not followed by two characters: ", s));
}

// parseHost parses host as an authority without user
// information. That is, as host[:port].
UrlError Url::parseHost(absl::string_view host) {
	UrlError err;

	if(absl::StartsWith(host, "[")) {
		// Parse an IP-Literal in RFC 3986 and RFC 6874.
		// E.g., "[fe80::1]", "[fe80::1%25en0]", "[fe80::1]:80".
		auto i = host.find_first_of("]");
		if(i == 0) {
			return Url::UrlParseError("missing ']' in host");
		}
		absl::string_view colonPort = host.substr(i);
		if(!internal::validOptionalPort(colonPort)) {
			return Url::UrlParseError(absl::StrCat("invalid port ", colonPort, " after host"));
		}

		// RFC 6874 defines that %25 (%-encoded percent) introduces
		// the zone identifier, and the zone identifier can use basically
		// any %-encoding it likes. That's different from the host, which
		// can only %-encode non-ASCII bytes.
		// We do impose some restrictions on the zone, to avoid stupidity
		// like newlines.
		auto host0 = host.substr(0,i);
		auto zone = host.find_first_of("%25");
		if(zone != host0.npos) {
			std::string host1;
			std::string host2;
			std::string host3;
			std::tie(host1, err) = unescape(host.substr(0,zone), internal::encoding::encodeHost);
			if(err != UrlNoError) {
				return err;
			}
			std::tie(host2, err) = unescape(host.substr(zone, i), internal::encoding::encodeZone);
			if(err != UrlNoError) {
				return err;
			}
			std::tie(host3, err) = unescape(host.substr(i), internal::encoding::encodeHost);
			if(err != UrlNoError) {
				return err;
			}
			mHost = absl::StrCat(host1, host2, host3);
			return UrlNoError;
		}
	}
	auto i = host.find_first_of(':'); 
	if(i == host.npos) {
		absl::string_view colonPort = host.substr(i);
		if(!internal::validOptionalPort(colonPort)) {
			return Url::UrlParseError(absl::StrCat("invalid port ", colonPort, " after host"));
		}
	}

	std::tie(host, err) = unescape(host, internal::encoding::encodeHost);
	if(err != UrlNoError) {
		return err;
	}

	mHost = (std::string)host;
	return UrlNoError;
}

/** parse parses a URL from a string in one of two contexts. If
 * viaRequest is true, the URL is assumed to have arrived via an HTTP request,
 * in which case only absolute URLs or path-absolute relative URLs are allowed.
 * If viaRequest is false, all forms of relative URLs are allowed.
 */
UrlError Url::parse(absl::string_view rawurl, bool viaRequest)
{
	absl::string_view rest;
	UrlError err;

	if(strings::containsCtlByte(rawurl)) {
		return Url::UrlParseError("net/url: invalid control character in URL");
	}

	if(rawurl.empty() && viaRequest) {
		return Url::UrlParseError("net/url: empty url");
	}

	if(rawurl == "*") {
		mPath = "*";
		return UrlNoError;
	}

	// Split off possible leading "http:", "mailto:", etc.
	// Cannot contain escaped characters.
	absl::string_view scheme;
    std::tie(scheme, rest, err) = internal::parseScheme(rawurl);
	if(err != UrlNoError) {
		return err;
	}
	mScheme = absl::AsciiStrToLower(scheme);

	if(absl::EndsWith(rest, "?") && strings::count(rest, "?") == 1) {
		mForceQuery = true;
		rest = rest.substr(0, rest.length()-1);
	} else {
		absl::string_view rawQuery;
		std::tie(rest, rawQuery) = internal::split(rest, "?", true);
		mRawQuery = (std::string)rawQuery;
	}

	if(!absl::StartsWith(rest, "/")) {
		if(mScheme.empty()) {
			// We consider rootless paths per RFC 3986 as opaque.
			mOpaque = (std::string)rest;
			return UrlNoError;
		}
		if(viaRequest) {
			return Url::UrlParseError("invalid URI for request");
		}

		// Avoid confusion with malformed schemes, like cache_object:foo/bar.
		// See golang.org/issue/16822.
		//
		// RFC 3986, §3.3:
		// In addition, a URI reference (Section 4.1) may be a relative-path reference,
		// in which case the first path segment cannot contain a colon (":") character.
		auto colon = rest.find_first_of(':');
		auto slash = rest.find_first_of('/');
		if(colon >= 0 && (slash < 0 || colon < slash) ) {
			// First path segment has colon. Not allowed in relative URL.
			return Url::UrlParseError("first path segment in URL cannot contain colon");
		}
	}

	if (mScheme.empty() || !viaRequest && !absl::StartsWith(rest, "///") && absl::StartsWith(rest, "//") ) {
		absl::string_view authority;
		std::tie(authority, rest) = internal::split(rest.substr(2), "/", false);
		err = parseAuthority(authority);
		if(err != UrlNoError) {
			return err;
		}
	}
	// Set Path and, optionally, RawPath.
	// RawPath is a hint of the encoding of Path. We don't want to set it if
	// the default escaping of Path is equivalent, to help make sure that people
	// don't rely on it in general.
	err = setPath(rest);
	if(err != UrlNoError) {
		return err;
	}
	return UrlNoError;
}

UrlError Url::parseAuthority(absl::string_view authority) {
	auto i = authority.find_last_of('@');
	UrlError err;
	if(i == authority.npos) {
		err = parseHost(authority);
	} else {
		err = parseHost(authority.substr(i+1));
	}
	if(err != UrlNoError) {
		return err;
	}
	if(i == authority.npos) {
		return UrlNoError;
	}
	absl::string_view userinfo = authority.substr(0,i);
	if(!internal::validUserinfo(userinfo)) {
		return Url::UrlParseError("net/url: invalid userinfo");
	}
	if(!absl::StrContains(userinfo, ":")) {
		std::tie(userinfo, err) = unescape(userinfo, internal::encoding::encodeUserPassword);
		if(err != UrlNoError) {
			return err;
		}
		mUsername = (std::string)userinfo;
	} else {
		absl::string_view username;
		absl::string_view password;
		std::tie(username, password) = internal::split(userinfo, ":", true);
		std::tie(username, err) = unescape(username, internal::encoding::encodeUserPassword);
		if(err != UrlNoError) {
			return err;
		}
		std::tie(password, err) = unescape(password, internal::encoding::encodeUserPassword);
		if(err != UrlNoError) {
			return err;
		}
		mUsername = (std::string)username;
		mPassword = (std::string)password;
	}
	return UrlNoError;
}

std::string to_string(Url url) {
    return url.toString();
}

}

}