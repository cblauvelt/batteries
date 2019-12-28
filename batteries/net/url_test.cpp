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

#include "url.h"
#include <absl/strings/str_replace.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using encoding = batteries::net::internal::encoding;
using QueryMap = batteries::net::internal::QueryMap;
using UrlError = batteries::net::UrlError;

// Test shouldEscape

struct ShouldEscapeTest {
    unsigned char in;
    batteries::net::internal::encoding encoding;
    bool escape;
};

std::ostream & operator<<(std::ostream &os, const ShouldEscapeTest& set)
{
    return os << "in: " << set.in << ", encoding: " << (int)set.encoding << ", escape:" << set.escape;
}

class MultipleShouldEscapeTests : public ::testing::TestWithParam<ShouldEscapeTest>{

};

TEST_P(MultipleShouldEscapeTests, Unescape) {
    // Test Queries
    bool result = batteries::net::internal::shouldEscape(GetParam().in, GetParam().encoding);
    EXPECT_EQ(GetParam().escape, result);
}

INSTANTIATE_TEST_CASE_P(
    ShouldEscapeTest,
    MultipleShouldEscapeTests,
    ::testing::Values(
        // Unreserved characters (§2.3)
        ShouldEscapeTest{'a', encoding::encodePath, false},
        ShouldEscapeTest{'a', encoding::encodeUserPassword, false},
        ShouldEscapeTest{'a', encoding::encodeQueryComponent, false},
        ShouldEscapeTest{'a', encoding::encodeFragment, false},
        ShouldEscapeTest{'a', encoding::encodeHost, false},
        ShouldEscapeTest{'z', encoding::encodePath, false},
        ShouldEscapeTest{'A', encoding::encodePath, false},
        ShouldEscapeTest{'Z', encoding::encodePath, false},
        ShouldEscapeTest{'0', encoding::encodePath, false},
        ShouldEscapeTest{'9', encoding::encodePath, false},
        ShouldEscapeTest{'/', encoding::encodePath, false},
        ShouldEscapeTest{'-', encoding::encodePath, false},
        ShouldEscapeTest{'-', encoding::encodeUserPassword, false},
        ShouldEscapeTest{'-', encoding::encodeQueryComponent, false},
        ShouldEscapeTest{'-', encoding::encodeFragment, false},
        ShouldEscapeTest{'.', encoding::encodePath, false},
        ShouldEscapeTest{'_', encoding::encodePath, false},
        ShouldEscapeTest{'~', encoding::encodePath, false},

        // User information (§3.2.1)
        ShouldEscapeTest{':', encoding::encodeUserPassword, true},
        ShouldEscapeTest{'/', encoding::encodeUserPassword, true},
        ShouldEscapeTest{'?', encoding::encodeUserPassword, true},
        ShouldEscapeTest{'@', encoding::encodeUserPassword, true},
        ShouldEscapeTest{'$', encoding::encodeUserPassword, false},
        ShouldEscapeTest{'&', encoding::encodeUserPassword, false},
        ShouldEscapeTest{'+', encoding::encodeUserPassword, false},
        ShouldEscapeTest{',', encoding::encodeUserPassword, false},
        ShouldEscapeTest{';', encoding::encodeUserPassword, false},
        ShouldEscapeTest{'=', encoding::encodeUserPassword, false},

        // Host (IP address, IPv6 address, registered name, port suffix; §3.2.2)
        ShouldEscapeTest{'!', encoding::encodeHost, false},
        ShouldEscapeTest{'$', encoding::encodeHost, false},
        ShouldEscapeTest{'&', encoding::encodeHost, false},
        ShouldEscapeTest{'\'', encoding::encodeHost, false},
        ShouldEscapeTest{'(', encoding::encodeHost, false},
        ShouldEscapeTest{')', encoding::encodeHost, false},
        ShouldEscapeTest{'*', encoding::encodeHost, false},
        ShouldEscapeTest{'+', encoding::encodeHost, false},
        ShouldEscapeTest{',', encoding::encodeHost, false},
        ShouldEscapeTest{';', encoding::encodeHost, false},
        ShouldEscapeTest{'=', encoding::encodeHost, false},
        ShouldEscapeTest{':', encoding::encodeHost, false},
        ShouldEscapeTest{'[', encoding::encodeHost, false},
        ShouldEscapeTest{']', encoding::encodeHost, false},
        ShouldEscapeTest{'0', encoding::encodeHost, false},
        ShouldEscapeTest{'9', encoding::encodeHost, false},
        ShouldEscapeTest{'A', encoding::encodeHost, false},
        ShouldEscapeTest{'z', encoding::encodeHost, false},
        ShouldEscapeTest{'_', encoding::encodeHost, false},
        ShouldEscapeTest{'-', encoding::encodeHost, false},
        ShouldEscapeTest{'.', encoding::encodeHost, false}
    )
);

// Test unescape

struct EscapeTest {
    std::string in;
    std::string out;
    UrlError error;
};

std::ostream & operator<<(std::ostream &os, const EscapeTest& et)
{
    return os << "in: " << et.in << ", out: " << et.out << ", error:" << et.error.message();
}

class MultipleUnescapeTests : public ::testing::TestWithParam<EscapeTest>{

};

TEST_P(MultipleUnescapeTests, Unescape) {
    std::string path;
    UrlError err;

    // Test Queries
    std::tie(path, err) = batteries::net::unescapeQuery(GetParam().in);
    EXPECT_EQ(GetParam().error.message(), err.message());
    EXPECT_EQ(GetParam().error.errorCode(), err.errorCode());
    EXPECT_EQ(GetParam().out, path);

    // Test Paths - Requires a special case for '+'
    path = absl::StrReplaceAll(GetParam().in, {{"+", "%20"}});
    std::tie(path, err) = batteries::net::unescapePath(path);
    EXPECT_EQ(GetParam().error.message(), err.message());
    EXPECT_EQ(GetParam().error.errorCode(), err.errorCode());
    EXPECT_EQ(GetParam().out, path);
}

INSTANTIATE_TEST_CASE_P(
    EscapeTests,
    MultipleUnescapeTests,
    ::testing::Values(
        EscapeTest{
            "",
            "",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "abc",
            "abc",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "1%41",
            "1A",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "1%41%42%43",
            "1ABC",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "%4a",
            "J",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "%6F",
            "o",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "%",
            "",
            batteries::net::UrlRangeError("%")
        },
        EscapeTest{
            "%a",
            "",
            batteries::net::UrlRangeError("%a")
        },
        EscapeTest{
            "%1",
            "",
            batteries::net::UrlRangeError("%1")
        },
        EscapeTest{
            "123%45%6",
            "",
            batteries::net::UrlRangeError("123%45%6")
        },
        EscapeTest{
            "%zzzzz",
            "",
            batteries::net::UrlEscapeError("%zz")
        },
        EscapeTest{
            "a+b",
            "a b",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "a%20b",
            "a b",
            batteries::net::UrlNoError
        }
    )
);

// Test escapeQuery

class MultipleEscapeQueryTests : public ::testing::TestWithParam<EscapeTest>{

};

TEST_P(MultipleEscapeQueryTests, EscapeQuery) {
    std::string path = batteries::net::escapeQuery(GetParam().in);
    EXPECT_EQ(GetParam().out, path);
}

INSTANTIATE_TEST_CASE_P(
    EscapeTests,
    MultipleEscapeQueryTests,
    ::testing::Values(
        EscapeTest{
            "",
            "",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "abc",
            "abc",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "one two",
	    	"one+two",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "10%",
		    "10%25",
            batteries::net::UrlNoError
        },
        EscapeTest{
            " ?&=#+%!<>#\"{}|\\^[]`☺\t:/@$'()*,;",
		    "+%3F%26%3D%23%2B%25%21%3C%3E%23%22%7B%7D%7C%5C%5E%5B%5D%60%E2%98%BA%09%3A%2F%40%24%27%28%29%2A%2C%3B",
            batteries::net::UrlNoError
        }
    )
);

// Test escapePath

class MultipleEscapePathTests : public ::testing::TestWithParam<EscapeTest>{

};

TEST_P(MultipleEscapePathTests, EscapePath) {
    std::string path = batteries::net::escapePath(GetParam().in);
    EXPECT_EQ(GetParam().out, path);
}

INSTANTIATE_TEST_CASE_P(
    EscapeTests,
    MultipleEscapePathTests,
    ::testing::Values(
        EscapeTest{
            "",
            "",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "abc",
            "abc",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "abc+def",
	    	"abc+def",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "a/b",
		    "a%2Fb",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "one two",
		    "one%20two",
            batteries::net::UrlNoError
        },
        EscapeTest{
            "10%",
		    "10%25",
            batteries::net::UrlNoError
        },
        EscapeTest{
            " ?&=#+%!<>#\"{}|\\^[]`☺\t:/@$'()*,;",
		    "%20%3F&=%23+%25%21%3C%3E%23%22%7B%7D%7C%5C%5E%5B%5D%60%E2%98%BA%09:%2F@$%27%28%29%2A%2C%3B",
            batteries::net::UrlNoError
        }
    )
);

struct ParseHostTest {
    std::string in;
    std::string host;
    std::string port;
    UrlError err;
};

std::ostream & operator<<(std::ostream &os, const ParseHostTest& pht)
{
    return os << "in: " << pht.in << ", host: " << pht.host << ", port:" << pht.port;
}

class MultipleParseHostTests : public ::testing::TestWithParam<ParseHostTest>{

};

TEST_P(MultipleParseHostTests, ParseHostTest) {
    std::string host, port;
    UrlError err;
    std::tie(host, port, err) = batteries::net::internal::parseHost(GetParam().in);
    EXPECT_EQ(GetParam().err, err);
    EXPECT_EQ(GetParam().host, host);
    EXPECT_EQ(GetParam().port, port);
}

INSTANTIATE_TEST_CASE_P(
    ParseHostTest,
    MultipleParseHostTests,
    ::testing::Values(
        ParseHostTest{"foo.com:80", "foo.com", "80", batteries::net::UrlNoError},
        ParseHostTest{"foo.com", "foo.com", "", batteries::net::UrlNoError},
		ParseHostTest{"foo.com:", "foo.com", "", batteries::net::UrlNoError},
		ParseHostTest{"FOO.COM", "FOO.COM", "", batteries::net::UrlNoError}, // no canonicalization
		ParseHostTest{"1.2.3.4", "1.2.3.4", "", batteries::net::UrlNoError},
		ParseHostTest{"1.2.3.4:80", "1.2.3.4", "80", batteries::net::UrlNoError},
		ParseHostTest{"[1:2:3:4]", "[1:2:3:4]", "", batteries::net::UrlNoError},
		ParseHostTest{"[1:2:3:4]:80", "[1:2:3:4]", "80", batteries::net::UrlNoError},
		ParseHostTest{"[::1]:80", "[::1]", "80", batteries::net::UrlNoError},
		ParseHostTest{"[::1]", "[::1]", "", batteries::net::UrlNoError},
		ParseHostTest{"[::1]:", "[::1]", "", batteries::net::UrlNoError},
		ParseHostTest{"[fe80::1%25en0]", "[fe80::1%en0]", "", batteries::net::UrlNoError},
		ParseHostTest{"[fe80::1%25en0]:8080", "[fe80::1%en0]", "8080", batteries::net::UrlNoError},
		ParseHostTest{"[fe80::1%25%65%6e%301-._~]", "[fe80::1%en01-._~]", "", batteries::net::UrlNoError},
		ParseHostTest{"[fe80::1%25%65%6e%301-._~]:8080", "[fe80::1%en01-._~]", "8080", batteries::net::UrlNoError},
		ParseHostTest{"localhost", "localhost", "", batteries::net::UrlNoError},
		ParseHostTest{"localhost:443", "localhost", "443", batteries::net::UrlNoError},
		ParseHostTest{"some.super.long.domain.example.org:8080", "some.super.long.domain.example.org", "8080"},
		ParseHostTest{"[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:17000", "[2001:0db8:85a3:0000:0000:8a2e:0370:7334]", "17000"},
		ParseHostTest{"[2001:0db8:85a3:0000:0000:8a2e:0370:7334]", "[2001:0db8:85a3:0000:0000:8a2e:0370:7334]", ""}
    )
);

struct ParseAuthorityTest {
    std::string in;
    std::string username;
    std::string password;
    absl::string_view host;
    UrlError err;
};

std::ostream & operator<<(std::ostream &os, const ParseAuthorityTest& pat)
{
    return os << "in: " << pat.in
              << ", username: " << pat.username
              << ", password: " << pat.password
              << ", host: " << pat.host
              << ", err: " << pat.err;
}

class MultipleParseAuthorityTests : public ::testing::TestWithParam<ParseAuthorityTest>{

};

TEST_P(MultipleParseAuthorityTests, ParseAuthorityTests) {
    std::string username, password;
    absl::string_view host;
    UrlError err;
    std::tie(username, password, host, err) = batteries::net::internal::parseAuthority(GetParam().in);
    EXPECT_EQ(GetParam().username, username);
    EXPECT_EQ(GetParam().password, password);
    EXPECT_EQ(GetParam().host, host);
    EXPECT_EQ(GetParam().err, err);
}

INSTANTIATE_TEST_CASE_P(
    ParseAuthorityTest,
    MultipleParseAuthorityTests,
    ::testing::Values(
        ParseAuthorityTest{
            "user:password@foo.com:80",
            "user",
            "password",
            "foo.com:80",
            batteries::net::UrlNoError
        },
        ParseAuthorityTest{
            "foo.com:80",
            "",
            "",
            "foo.com:80",
            batteries::net::UrlNoError
        },
        ParseAuthorityTest{ // malformed host should not cause error
            "user@password:foo.com:80",
            "user",
            "",
            "password:foo.com:80",
            batteries::net::UrlNoError
        },
        ParseAuthorityTest{
            "us^er:password@foo.com:80",
            "",
            "",
            "",
            batteries::net::UrlParseError("invalid userinfo")
        }
    )
);

struct ParseSchemeTest {
    std::string in;
    std::string scheme;
    absl::string_view rest;
    UrlError err;
};

std::ostream & operator<<(std::ostream &os, const ParseSchemeTest& pst)
{
    return os << "in: " << pst.in
              << ", scheme: " << pst.scheme
              << ", rest: " << pst.rest
              << ", err: " << pst.err;
}

class MultipleParseSchemeTests : public ::testing::TestWithParam<ParseSchemeTest>{

};

TEST_P(MultipleParseSchemeTests, ParseSchemeTests) {
    std::string scheme;
    absl::string_view rest;
    UrlError err;
    std::tie(scheme, rest, err) = batteries::net::internal::parseScheme(GetParam().in);
    EXPECT_EQ(GetParam().scheme, scheme);
    EXPECT_EQ(GetParam().rest, rest);
    EXPECT_EQ(GetParam().err, err);
}

INSTANTIATE_TEST_CASE_P(
    ParseSchemeTest,
    MultipleParseSchemeTests,
    ::testing::Values(
        ParseSchemeTest{
            "http://root:password@192.168.1.1:8080/path/to/file",
            "http",
            "//root:password@192.168.1.1:8080/path/to/file",
            batteries::net::UrlNoError
        },
        ParseSchemeTest{
            "file://test.txt",
            "file",
            "//test.txt",
            batteries::net::UrlNoError
        }
    )
);

struct ParseUrlTest {
    std::string in;
    std::string scheme;
    std::string opaque;
    std::string username;
    std::string password;
    std::string host;
    std::string hostname;
    std::string port;
    std::string path;
    std::string rawPath;
    batteries::net::Query query;
    std::string fragment;
    batteries::net::UrlError err;
};

std::ostream & operator<<(std::ostream &os, const ParseUrlTest& pt)
{
    return os << "in: " << pt.in
              << ", scheme: " << pt.scheme
              << ", opaque: " << pt.opaque
              << ", username: " << pt.username
              << ", password: " << pt.password
              << ", host: " << pt.host
              << ", hostname: " << pt.hostname
              << ", port: " << pt.port
              << ", path: " << pt.path
              << ", rawPath: " << pt.rawPath
              << ", query: " << pt.query.toString()
              << ", fragment: " << pt.fragment
              << ", err: " << pt.err;
}

class MultipleParseUrlTests : public ::testing::TestWithParam<ParseUrlTest>{
    protected:
        batteries::net::Url url;
};

TEST_P(MultipleParseUrlTests, ParseUrlTests) {
    batteries::net::UrlError err = url.parse(GetParam().in);
    EXPECT_EQ(GetParam().err, err);
    EXPECT_EQ(GetParam().scheme, url.scheme());
    EXPECT_EQ(GetParam().opaque, url.opaque());
    EXPECT_EQ(GetParam().username, url.username());
    EXPECT_EQ(GetParam().password, url.password());
    EXPECT_EQ(GetParam().host, url.host());
    EXPECT_EQ(GetParam().hostname, url.hostname());
    EXPECT_EQ(GetParam().port, url.port());
    EXPECT_EQ(GetParam().path, url.path());
    EXPECT_EQ(GetParam().rawPath, url.rawPath());
    EXPECT_EQ(GetParam().query, url.query());
    EXPECT_EQ(GetParam().fragment, url.fragment());
}

INSTANTIATE_TEST_CASE_P(
    ParseUrlTest,
    MultipleParseUrlTests,
    ::testing::Values(
        ParseUrlTest{
            "http://foo.com",
            "http",
            "",
            "",
            "",
            "foo.com",
            "foo.com",
            "",
            "",
            "",
            batteries::net::Query(),
            "",
            batteries::net::UrlNoError
        },
        ParseUrlTest{
            "http://foo.com/",
            "http",
            "",
            "",
            "",
            "foo.com",
            "foo.com",
            "",
            "/",
            "",
            batteries::net::Query(),
            "",
            batteries::net::UrlNoError
        },
        ParseUrlTest{
            "http://foo.com/path",
            "http",
            "",
            "",
            "",
            "foo.com",
            "foo.com",
            "",
            "/path",
            "",
            batteries::net::Query(),
            "",
            batteries::net::UrlNoError
        },
        ParseUrlTest{
            "http://foo.com/path%20escaped",
            "http",
            "",
            "",
            "",
            "foo.com",
            "foo.com",
            "",
            "/path escaped",
            "/path%20escaped",
            batteries::net::Query(),
            "",
            batteries::net::UrlNoError
        },
        ParseUrlTest{
            "http://foo.com/path?a=1&b=2",
            "http",
            "",
            "",
            "",
            "foo.com",
            "foo.com",
            "",
            "/path",
            "",
            batteries::net::Query(batteries::net::QueryValues{{"a", "1"}, {"b", "2"}}),
            "",
            batteries::net::UrlNoError
        },
        ParseUrlTest{
            "/", // in
            "", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "*", // in
            "", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "*", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://192.168.0.1:8080/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "192.168.0.1:8080", // host
            "192.168.0.1", // hostname
            "8080", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://192.168.0.1:8080/?a=1&b=2", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "192.168.0.1:8080", // host
            "192.168.0.1", // hostname
            "8080", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(batteries::net::QueryValues{{"a", "1"}, {"b", "2"}}), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://[fe80::1]/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "[fe80::1]", // host
            "[fe80::1]", // hostname
            "", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://[fe80::1]:8080/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "[fe80::1]:8080", // host
            "[fe80::1]", // hostname
            "8080", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://[fe80::1]:8080/?a=1&b=2", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "[fe80::1]:8080", // host
            "[fe80::1]", // hostname
            "8080", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(batteries::net::QueryValues{{"a", "1"}, {"b", "2"}}), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://root:password@192.168.1.1:8080/path/to/file#nonsense",
            "http",
            "",
            "root",
            "password",
            "192.168.1.1:8080",
            "192.168.1.1",
            "8080",
            "/path/to/file",
            "",
            batteries::net::Query(),
            "nonsense",
            batteries::net::UrlNoError
        },
        ParseUrlTest{
            "http://[fe80::1%25en0]/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "[fe80::1%en0]", // host
            "[fe80::1%en0]", // hostname
            "", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://[fe80::1%25en0]:8080/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "[fe80::1%en0]:8080", // host
            "[fe80::1%en0]", // hostname
            "8080", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://[fe80::1%25%65%6e%301-._~]/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "[fe80::1%en01-._~]", // host
            "[fe80::1%en01-._~]", // hostname
            "", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://[fe80::1%25%65%6e%301-._~]:8080/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "[fe80::1%en01-._~]:8080", // host
            "[fe80::1%en01-._~]", // hostname
            "8080", // port
            "/", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "foo.html", // in
            "", // scheme
            "foo.html", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "../dir/", // in
            "", // scheme
            "../dir/", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlNoError // error
        },
        ParseUrlTest{
            "http://192.168.0.%31/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlEscapeError("%31") // error
        },
        ParseUrlTest{
            "http://192.168.0.%31:8080/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlEscapeError("%31") // error
        },
        ParseUrlTest{
            "http://[fe80::%31]/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlEscapeError("%31]") // error
        },
        ParseUrlTest{
            "http://[fe80::%31]:8080/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlEscapeError("%31]") // error
        },
        ParseUrlTest{
            "http://[fe80::%31%25en0]/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlEscapeError("%31") // error
        },
        ParseUrlTest{
            "http://[fe80::%31%25en0]:8080/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlEscapeError("%31") // error
        },
        // These two cases are valid as textual representations as
        // described in RFC 4007, but are not valid as address
        // literals with IPv6 zone identifiers in URIs as described in
        // RFC 6874.
        ParseUrlTest{
            "http://[fe80::1%en0]/", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlEscapeError("%en0]") // error
        },
        ParseUrlTest{
            "http://[fe80::1%en0]:8080//", // in
            "http", // scheme
            "", // opaque
            "", // username
            "", // password
            "", // host
            "", // hostname
            "", // port
            "", // path
            "", // rawPath
            batteries::net::Query(), // query
            "", // fragment
            batteries::net::UrlEscapeError("%en0]") // error
        }
    )
);

using RoundTripTest = std::string;

class MultipleRoundTripTests : public ::testing::TestWithParam<RoundTripTest>{
    protected:
        batteries::net::Url url;
};

TEST_P(MultipleRoundTripTests, RoundTripTests) {
    batteries::net::UrlError err = url.parse(GetParam());
    EXPECT_EQ(err, batteries::net::UrlNoError);
    EXPECT_EQ(GetParam(), url.toString());
    
}

INSTANTIATE_TEST_CASE_P(
    RoundTripTest,
    MultipleRoundTripTests,
    ::testing::Values(
        RoundTripTest{
            "http://foo.com"
        },
        RoundTripTest{
            "http://foo.com/"
        },
        RoundTripTest{
            "http://foo.com/path/to/file"
        },
        RoundTripTest{
            "http://foo.com/path%20escaped"
        },
        RoundTripTest{
            "http://foo.com/path?a=1&b=2"
        },
        RoundTripTest{
            "/", // in
        },
        RoundTripTest{
            "*", // in
        },
        RoundTripTest{
            "http://192.168.0.1:8080/"
        },
        RoundTripTest{
            "http://192.168.0.1:8080/?a=1&b=2"
        },
        RoundTripTest{
            "http://[fe80::1]/"
        },
        RoundTripTest{
            "http://[fe80::1]:8080/"
        },
        RoundTripTest{
            "http://[fe80::1]:8080/?a=1&b=2"
        },
        RoundTripTest{
            "http://root:password@192.168.1.1:8080/path/to/file#nonsense"
        },
        RoundTripTest{
            "http://[fe80::1%25en0]/"
        },
        RoundTripTest{
            "http://[fe80::1%25en0]:8080/"
        }
    )
);

TEST(InitTest, InitTest) {
    std::string urlString = "http://root:password@192.168.1.1:8080/path/to/file#nonsense";
    batteries::net::Url url1;
    batteries::net::Url url2(urlString);
    url1.parse(urlString);

    EXPECT_EQ(url1, url2);
}

} // namespace
