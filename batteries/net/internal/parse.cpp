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

#include <tuple>

#include <absl/strings/ascii.h>
#include <absl/strings/str_split.h>
#include <absl/strings/match.h>

#include "parse.h"

namespace batteries {

namespace net {

namespace internal {

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

bool validOptionalPort(absl::string_view port) {
	// No port is a valid port
	if(port.empty()) { return true;}

    if(port.at(0) != ':') { return false; }
	for(auto c : port.substr(1)) {
		if(!absl::ascii_isdigit(c)) { return false;}
	}

	return true;
}

bool validUserinfo(absl::string_view s) {
	for(auto c : s) {
		if(!absl::ascii_isalnum(c) && c != '-' && c!= '.' && c!= '_' &&
			c!= ':' && c!= '~' && c!= '!' && c!= '$' && c!= '&' &&
			c!= '\'' && c!= '(' && c!= ')' && c!= '*' &&
			c!= '+' && c!= ',' && c!= ';' && c!= '=' && c!= '%' && c!= '@')
		{
			return false;
		}
	}

	return true;
}

std::tuple<std::string, absl::string_view, UrlError>
parseFragment(absl::string_view rawurl) {
    std::string fragment;
    absl::string_view fragment_view, rest;
    UrlError err;
    
	std::tie(rest, fragment_view) = split(rawurl, "#", true);
    if(fragment_view.empty()) {
        return std::make_tuple(
            fragment,
            rest,
            err
        );
    }

    std::tie(fragment, err) = unescape(fragment_view, encoding::encodeFragment);
    if(err != UrlNoError) {
        return std::make_tuple(
            std::string(),
            rest,
            err
        );
    }

    return std::make_tuple(
        fragment,
        rest,
        err
    );
}

std::tuple<std::string, absl::string_view, UrlError>
parseScheme(absl::string_view rawurl) {
	for(int i = 0; i < rawurl.length(); i++) {
		byte c = rawurl[i];
		if(absl::ascii_isalpha(c)) { continue;} // do nothing
		
        if(absl::ascii_isdigit(c) || c == '+' || c == '-' || c == '.') {
			if(i == 0) {
				return std::make_tuple(
                    "",
                    rawurl,
                    UrlNoError
                );
			}
        }
		if(c == ':') {
			if(i == 0) {
				return std::make_tuple(
                    "",
                    absl::string_view(),
                    UrlParseError("missing protocol scheme")
                );
			}
            return std::make_tuple(
                (std::string)rawurl.substr(0,i),
                rawurl.substr(i+1),
                UrlNoError
            );
        }
		
        // we have encountered an invalid character,
        // so there is no valid scheme
        return std::make_tuple(
            "", rawurl, UrlNoError
        );
	}
    return std::make_tuple(
        "", rawurl, UrlNoError
    );
}

std::tuple<std::string, std::string, absl::string_view,UrlError>
parseAuthority(absl::string_view authority) {
	auto i = authority.rfind('@');
	
    if(i == authority.npos) {
		return std::make_tuple(
            "",
            "",
            authority,
            UrlNoError
        );
	}

	absl::string_view userinfo = authority.substr(0,i);

	if(!internal::validUserinfo(userinfo)) {
		return std::make_tuple(
            "",
            "",
            absl::string_view(),
            UrlParseError("invalid userinfo")
        );
	}

    // Return values
    std::string username;
    std::string password;
    absl::string_view host = authority.substr(i+1);
	UrlError err;

    // Has no password
	if(!absl::StrContains(userinfo, ":")) {
		std::tie(userinfo, err) = unescape(userinfo, internal::encoding::encodeUserPassword);
		if(err != UrlNoError) {
			return std::make_tuple(
                "",
                "",
                host,
                err
            );
		}
		username = (std::string)userinfo;
	} else { // Has password
        absl::string_view username_view;
        absl::string_view password_view;
		std::tie(username_view, password_view) = internal::split(userinfo, ":", true);
		std::tie(username, err) = unescape(username_view, internal::encoding::encodeUserPassword);
		if(err != UrlNoError) {
			return std::make_tuple(
                "",
                "",
                host,
                err
            );
		}
		std::tie(password, err) = unescape(password_view, internal::encoding::encodeUserPassword);
		if(err != UrlNoError) {
			return std::make_tuple(
                "",
                "",
                host,
                err
            );
		}
	}
	return std::make_tuple(
        username,
        password,
        host,
        err
    );
}

std::tuple<std::string, std::string, UrlError> parseHost(absl::string_view host) {
    UrlError err;
    std::string hostString;
    std::string portString;
    absl::string_view port;

    // Handle IPv6
	if(absl::StartsWith(host, "[")) {
		// Parse an IP-Literal in RFC 3986 and RFC 6874.
		// E.g., "[fe80::1]", "[fe80::1%25en0]", "[fe80::1]:80".
		auto i = host.find("]");
		if(i == host.npos) {
			return std::make_tuple(
                "",
                "",
                UrlParseError("missing ']' in host")
            );
		}
		port = host.substr(i+1);
		if(!internal::validOptionalPort(port)) {
			return std::make_tuple(
                "",
                "",
                UrlParseError(absl::StrCat("invalid port ", port, " after host"))
            );
		}
        if(!port.empty()) {
            // Remove leading ':'
            portString = (std::string)port.substr(1);
        }

        // Remove port information and '[]' from the host
        host = host.substr(0, i+1);

		// RFC 6874 defines that %25 (%-encoded percent) introduces
		// the zone identifier, and the zone identifier can use basically
		// any %-encoding it likes. That's different from the host, which
		// can only %-encode non-ASCII bytes.
		// We do impose some restrictions on the zone, to avoid stupidity
		// like newlines.
		auto zone = host.find("%25");
		if(zone != host.npos) {
			std::string host1;
			std::string host2;
			std::string host3;
			std::tie(host1, err) = unescape(host.substr(0,zone), internal::encoding::encodeHost);
			if(err != UrlNoError) {
				return std::make_tuple(
                    "",
                    "",
                    err
                );
			}
			std::tie(host2, err) = unescape(host.substr(zone, i), internal::encoding::encodeZone);
			if(err != UrlNoError) {
				return std::make_tuple(
                    "",
                    "",
                    err
                );
			}
            hostString = absl::StrCat(host1, host2);

            return std::make_tuple(
                hostString,
                portString,
                UrlNoError
            );
		}
	} else {

        // Handle IPv4
        auto i = host.rfind(':'); 
        if(i != host.npos) { // Process with port number
            port = (std::string)host.substr(i);
            if(!internal::validOptionalPort(port)) {
                return std::make_tuple(
                    "",
                    "",
                    UrlParseError(absl::StrCat("invalid port ", port, " after host"))
                );
            }
            // Remove colon and assign to return value
            portString = (std::string)port.substr(1);
            // Remove port number from host string
            host = host.substr(0,i);
        }
    }

	std::tie(hostString, err) = unescape(host, internal::encoding::encodeHost);
	if(err != UrlNoError) {
		return std::make_tuple(
                "",
                "",
                err
        );
	}

	return std::make_tuple(
        hostString,
        portString,
        UrlNoError
    );
}

std::tuple<QueryMap, UrlError>
parseQuery(absl::string_view query) {
	UrlError err;
	QueryMap map;
	std::string key;
	std::string value;

	std::vector<absl::string_view> splitResults = absl::StrSplit(query, absl::ByAnyChar("&;"));
	for(auto& result : splitResults) {
		if(result.empty()) {
			return std::make_tuple(
				QueryMap(),
				UrlParseError(query)
			);
		}
		std::vector<absl::string_view> keyValueResults = absl::StrSplit(result, '=');
		if(keyValueResults.size() != 2) {
			return std::make_tuple(
				QueryMap(),
				UrlParseError(query)
			);
		}

		std::tie(key, err) = unescape(keyValueResults[0], encoding::encodeQueryComponent);
		if(err != UrlNoError) {
			break;
		}

		std::tie(value, err) = unescape(keyValueResults[1], encoding::encodeQueryComponent);
		if(err != UrlNoError) {
			break;
		}

		map.emplace(key, value);
	}
	

	return std::make_tuple(map, err);
}

}

}

}
