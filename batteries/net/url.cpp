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
	mPort(),
    mPath(),
    mRawPath(),
	mQuery(),
    mFragment()
{}

UrlError Url::parse(absl::string_view rawUrl) {
	parse(rawUrl, false);
}
UrlError Url::parseUri(absl::string_view rawUrl) {
	parse(rawUrl, true);
}

std::string Url::scheme() const {
	return mScheme;
}

void Url::setScheme(std::string scheme) {
	mScheme = scheme;
}

std::string Url::opaque() const {
	return mOpaque;
}

void Url::setOpaque(std::string opaque) {
	mOpaque = opaque;
}

std::string Url::username() const {
	return mUsername;
}

void Url::setUsername(std::string username) {
	mUsername = username;
}

std::string Url::password() const {
	return mPassword;
}

void Url::setPassword(std::string password) {
	mPassword = password;
}

std::string Url::host() const {
	return mHost;
}

UrlError Url::setHost(std::string host) {
	UrlError err;
	std::tie(mHost, mPort, err) = internal::parseHost(host);
	return err;
}

std::string Url::port() const {
	return mPort;
}

void Url::setPort(uint16_t port) {
	mPort = absl::StrCat(port);
}

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
	return urlString;
}

std::string Url::escapedPath() const {
    return internal::escape(mPath, internal::encoding::encodePathSegment);
}

std::string Url::escapedQuery() const {
	std::string query;
    return internal::escape(query, internal::encoding::encodeQueryComponent);
}

/** parse parses a URL from a string in one of two contexts. If
 * viaRequest is true, the URL is assumed to have arrived via an HTTP request,
 * in which case only absolute URLs or path-absolute relative URLs are allowed.
 * If viaRequest is false, all forms of relative URLs are allowed.
 */
UrlError Url::parse(absl::string_view rawurl, bool viaRequest) {
	absl::string_view rest;
	UrlError err;

	if(strings::containsCtlByte(rawurl)) {
		return UrlParseError("net/url: invalid control character in URL");
	}

	if(rawurl.empty() && viaRequest) {
		return UrlParseError("net/url: empty url");
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
		mQuery.setForceQuery(true);
		rest = rest.substr(0, rest.length()-1);
	} else {
		absl::string_view rawQuery;
		std::tie(rest, rawQuery) = internal::split(rest, "?", true);
		mQuery.parse((std::string)rawQuery);
	}

	if(!absl::StartsWith(rest, "/")) {
		if(mScheme.empty()) {
			// We consider rootless paths per RFC 3986 as opaque.
			mOpaque = (std::string)rest;
			return UrlNoError;
		}
		if(viaRequest) {
			return UrlParseError("invalid URI for request");
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
			return UrlParseError("first path segment in URL cannot contain colon");
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
		std::tie(mHost, mPort, err) = internal::parseHost(authority);
	} else {
		std::tie(mHost, mPort, err) = internal::parseHost(authority.substr(i+1));
	}
	if(err != UrlNoError) {
		return err;
	}
	if(i == authority.npos) {
		return UrlNoError;
	}
	absl::string_view userinfo = authority.substr(0,i);
	if(!internal::validUserinfo(userinfo)) {
		return UrlParseError("net/url: invalid userinfo");
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

}

}

namespace std {

std::string to_string(batteries::net::Url url) {
    return url.toString();
}

}