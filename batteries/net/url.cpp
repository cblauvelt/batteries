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

// TODO: Implement
Url ResolveReference(Url url) {
	return url;
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


Url::Url(std::string rawurl) :
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
{
	parse(rawurl);
}

UrlError Url::parse(absl::string_view rawUrl) {
	return parse(rawUrl, false);
}
UrlError Url::parseUri(absl::string_view rawUrl) {
	return parse(rawUrl, true);
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
	if(!mPort.empty()) {
		return absl::StrCat(mHost, ":", mPort);
	}

	return mHost;	
}

UrlError Url::setHost(std::string host) {
	UrlError err;
	std::tie(mHost, mPort, err) = internal::parseHost(host);
	return err;
}

std::string Url::hostname() const {
	return mHost;
}

void Url::setHostname(std::string hostname) {
	mHost = hostname;
}

std::string Url::port() const {
	return mPort;
}

void Url::setPort(uint16_t port) {
	mPort = absl::StrCat(port);
}

std::string Url::path() const {
	return mPath;
}

std::string Url::rawPath() const {
	return mRawPath;
}

UrlError Url::setPath(absl::string_view path) {
	UrlError err;
	std::tie(mPath, err) = internal::unescape(path, internal::encoding::encodePath);
	if(err != UrlNoError) {
		return err;
	}
	
	std::string escapedPath = internal::escape(mPath, internal::encoding::encodePath);
	if(mPath == escapedPath) {
		// Default encoding is fine.
		mRawPath = "";
	} else {
		mRawPath = (std::string)path;
	}
	return UrlNoError;
}

Query Url::query() const {
	return mQuery;
}

void Url::setQuery(const Query& query) {
	mQuery = query;
}

std::string Url::fragment() const {
	return internal::escape(mFragment, internal::encoding::encodeFragment);
}

UrlError Url::setFragment(std::string fragment) {
	UrlError err;
	std::tie(mFragment, err) = internal::unescape(fragment, internal::encoding::encodeFragment);
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
	std::ostringstream buf;

	if(mPath == "*") {
		return "*";
	}
    if(!mScheme.empty()) {
		buf << mScheme << ':';
	}
	if(!mOpaque.empty()) {
		buf << mOpaque;
	} else {
		if(!mScheme.empty() || !mHost.empty() || !mUsername.empty()) {
			if(!mHost.empty() || !mPath.empty() || !mUsername.empty()) {
				buf << "//";
			}
			if(!mUsername.empty()) {
				buf << mUsername;
				if(!mPassword.empty()) { buf << ":" << mPassword; }
				buf << '@';
			}
			if(!mHost.empty()) {
				buf << internal::escape(mHost, internal::encoding::encodeHost);
				if(!mPort.empty()) {
					buf << ":" << mPort;
				}
			}
		}
		auto path = escapedPath();
		if(!path.empty() && path[0] != '/' && !mHost.empty()) {
			buf << '/';
		}
		if(buf.tellp() == 0) {
			// RFC 3986 §4.2
			// A path segment that contains a colon character (e.g., "this:that")
			// cannot be used as the first segment of a relative-path reference, as
			// it would be mistaken for a scheme name. Such a segment must be
			// preceded by a dot-segment (e.g., "./this:that") to make a relative-
			// path reference.
			auto i = path.find(':');
			if(i != path.npos && path.substr(0,i).find('/') != path.npos) {
				buf << "./";
			}
		}
		buf << path;
	}
	
	buf << mQuery.toString();
	
	if(!mFragment.empty()) {
		buf << '#' << internal::escape(mFragment, internal::encoding::encodeFragment);
	}
	return buf.str();
}

std::string Url::requestUri() const {
	auto result = mOpaque;
	if(result == "") {
		result = escapedPath();
		if(result == "") {
			result = "/";
		}
	} else {
		if(absl::StartsWith(result, "//") ) {
			result = mScheme + ":" + result;
		}
	}
	absl::StrAppend(&result, mQuery.toString());
	
	return result;
}

std::string Url::escapedPath() const {
    return internal::escape(mPath, internal::encoding::encodePath);
}

std::string Url::escapedQuery() const {
	std::string query;
    return internal::escape(query, internal::encoding::encodeQueryComponent);
}

bool Url::operator==(const Url& rhs) const {
	return (
		mScheme == rhs.mScheme &&
		mOpaque == rhs.mOpaque &&
		mUsername == rhs.mUsername &&
		mPassword == rhs.mPassword &&
		mHost == rhs.mHost &&
		mPort == rhs.mPort &&
		mPath == rhs.mPath &&
		mRawPath == rhs.mRawPath &&
		mQuery == rhs.mQuery &&
		mFragment == rhs.mFragment
	);
}

bool Url::operator!=(const Url& rhs) const {
	return !(*this == rhs);
}

UrlError Url::parse(absl::string_view rawurl, bool viaRequest) {
	absl::string_view rest;
	UrlError err;

	if(strings::containsCtlChar(rawurl)) {
		return UrlParseError("invalid control character in URL");
	}

	if(rawurl.empty() && viaRequest) {
		return UrlParseError("empty url");
	}

	if(rawurl == "*") {
		mPath = "*";
		return UrlNoError;
	}

	// Split off fragment
	std::tie(mFragment, rest, err) = internal::parseFragment(rawurl);

	// Split off possible leading "http:", "mailto:", etc.
	// Cannot contain escaped characters.
	absl::string_view scheme;
    std::tie(scheme, rest, err) = internal::parseScheme(rest);
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
		auto colon = rest.find(':');
		auto slash = rest.find('/');
		if(colon >= 0 && (slash < 0 || colon < slash) ) {
			// First path segment has colon. Not allowed in relative URL.
			return UrlParseError("first path segment in URL cannot contain colon");
		}
	}

	if (!mScheme.empty() || !viaRequest && !absl::StartsWith(rest, "///") && absl::StartsWith(rest, "//") ) {
		absl::string_view authority;
		absl::string_view host;

		// Separate authority@host from the path
		std::tie(authority, rest) = internal::split(rest.substr(2), "/", false);

		// Parse the username and password
		std::tie(mUsername, mPassword, host, err) = internal::parseAuthority(authority);
		if(err != UrlNoError) {
			return err;
		}

		// Parse the host
		std::tie(mHost, mPort, err) = internal::parseHost(host);
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

}

}

namespace std {

std::string to_string(batteries::net::Url url) {
    return url.toString();
}

}