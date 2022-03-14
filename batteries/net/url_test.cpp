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

#include "url.hpp"
#include <absl/strings/str_replace.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using encoding = batteries::net::internal::encoding;
using query_map = batteries::net::internal::query_map;
using url_error = batteries::errors::error;
using batteries::net::url_error_code;

// Test shouldEscape

struct should_escape_test {
    unsigned char in;
    batteries::net::internal::encoding encoding;
    bool escape;
};

std::ostream& operator<<(std::ostream& os, const should_escape_test& set) {
    return os << "in: " << set.in << ", encoding: " << (int)set.encoding
              << ", escape:" << set.escape;
}

class MultipleShouldEscapeTests
    : public ::testing::TestWithParam<should_escape_test> {};

TEST_P(MultipleShouldEscapeTests, Unescape) {
    // Test Queries
    bool result = batteries::net::internal::shouldEscape(GetParam().in,
                                                         GetParam().encoding);
    EXPECT_EQ(GetParam().escape, result);
}

INSTANTIATE_TEST_SUITE_P(
    should_escape_test, MultipleShouldEscapeTests,
    ::testing::Values(
        // Unreserved characters (§2.3)
        should_escape_test{'a', encoding::encodePath, false},
        should_escape_test{'a', encoding::encodeUserPassword, false},
        should_escape_test{'a', encoding::encodeQueryComponent, false},
        should_escape_test{'a', encoding::encodeFragment, false},
        should_escape_test{'a', encoding::encodeHost, false},
        should_escape_test{'z', encoding::encodePath, false},
        should_escape_test{'A', encoding::encodePath, false},
        should_escape_test{'Z', encoding::encodePath, false},
        should_escape_test{'0', encoding::encodePath, false},
        should_escape_test{'9', encoding::encodePath, false},
        should_escape_test{'/', encoding::encodePath, false},
        should_escape_test{'-', encoding::encodePath, false},
        should_escape_test{'-', encoding::encodeUserPassword, false},
        should_escape_test{'-', encoding::encodeQueryComponent, false},
        should_escape_test{'-', encoding::encodeFragment, false},
        should_escape_test{'.', encoding::encodePath, false},
        should_escape_test{'_', encoding::encodePath, false},
        should_escape_test{'~', encoding::encodePath, false},

        // User information (§3.2.1)
        should_escape_test{':', encoding::encodeUserPassword, true},
        should_escape_test{'/', encoding::encodeUserPassword, true},
        should_escape_test{'?', encoding::encodeUserPassword, true},
        should_escape_test{'@', encoding::encodeUserPassword, true},
        should_escape_test{'$', encoding::encodeUserPassword, false},
        should_escape_test{'&', encoding::encodeUserPassword, false},
        should_escape_test{'+', encoding::encodeUserPassword, false},
        should_escape_test{',', encoding::encodeUserPassword, false},
        should_escape_test{';', encoding::encodeUserPassword, false},
        should_escape_test{'=', encoding::encodeUserPassword, false},

        // Host (IP address, IPv6 address, registered name, port suffix; §3.2.2)
        should_escape_test{'!', encoding::encodeHost, false},
        should_escape_test{'$', encoding::encodeHost, false},
        should_escape_test{'&', encoding::encodeHost, false},
        should_escape_test{'\'', encoding::encodeHost, false},
        should_escape_test{'(', encoding::encodeHost, false},
        should_escape_test{')', encoding::encodeHost, false},
        should_escape_test{'*', encoding::encodeHost, false},
        should_escape_test{'+', encoding::encodeHost, false},
        should_escape_test{',', encoding::encodeHost, false},
        should_escape_test{';', encoding::encodeHost, false},
        should_escape_test{'=', encoding::encodeHost, false},
        should_escape_test{':', encoding::encodeHost, false},
        should_escape_test{'[', encoding::encodeHost, false},
        should_escape_test{']', encoding::encodeHost, false},
        should_escape_test{'0', encoding::encodeHost, false},
        should_escape_test{'9', encoding::encodeHost, false},
        should_escape_test{'A', encoding::encodeHost, false},
        should_escape_test{'z', encoding::encodeHost, false},
        should_escape_test{'_', encoding::encodeHost, false},
        should_escape_test{'-', encoding::encodeHost, false},
        should_escape_test{'.', encoding::encodeHost, false}));

// Test unescape

struct EscapeTest {
    std::string in;
    std::string out;
    url_error error;
};

std::ostream& operator<<(std::ostream& os, const EscapeTest& et) {
    return os << "in: " << et.in << ", out: " << et.out
              << ", error:" << et.error.message();
}

class MultipleUnescapeTests : public ::testing::TestWithParam<EscapeTest> {};

TEST_P(MultipleUnescapeTests, Unescape) {
    std::string path;
    url_error err;

    // Test Queries
    std::tie(path, err) = batteries::net::unescape_query(GetParam().in);
    EXPECT_EQ(GetParam().error.message(), err.message());
    EXPECT_EQ(GetParam().error, err);
    EXPECT_EQ(GetParam().out, path);

    // Test Paths - Requires a special case for '+'
    path = absl::StrReplaceAll(GetParam().in, {{"+", "%20"}});
    std::tie(path, err) = batteries::net::unescape_path(path);
    EXPECT_EQ(GetParam().error.message(), err.message());
    EXPECT_EQ(GetParam().error, err);
    EXPECT_EQ(GetParam().out, path);
}

INSTANTIATE_TEST_SUITE_P(
    EscapeTests, MultipleUnescapeTests,
    ::testing::Values(
        EscapeTest{"", "", url_error{}}, EscapeTest{"abc", "abc", url_error{}},
        EscapeTest{"1%41", "1A", url_error{}},
        EscapeTest{"1%41%42%43", "1ABC", url_error{}},
        EscapeTest{"%4a", "J", url_error{}},
        EscapeTest{"%6F", "o", url_error{}},
        EscapeTest{"%", "", url_error(url_error_code::range_error, "%")},
        EscapeTest{"%a", "", url_error(url_error_code::range_error, "%a")},
        EscapeTest{"%1", "", url_error(url_error_code::range_error, "%1")},
        EscapeTest{"123%45%6", "",
                   url_error(url_error_code::range_error, "123%45%6")},
        EscapeTest{"%zzzzz", "",
                   url_error(url_error_code::escape_error, "%zz")},
        EscapeTest{"a+b", "a b", url_error{}},
        EscapeTest{"a%20b", "a b", url_error{}}));

// Test escape_query

class MultipleEscapeQueryTests : public ::testing::TestWithParam<EscapeTest> {};

TEST_P(MultipleEscapeQueryTests, EscapeQuery) {
    std::string path = batteries::net::escape_query(GetParam().in);
    EXPECT_EQ(GetParam().out, path);
}

INSTANTIATE_TEST_SUITE_P(
    EscapeTests, MultipleEscapeQueryTests,
    ::testing::Values(
        EscapeTest{"", "", url_error{}}, EscapeTest{"abc", "abc", url_error{}},
        EscapeTest{"one two", "one+two", url_error{}},
        EscapeTest{"10%", "10%25", url_error{}},
        EscapeTest{" ?&=#+%!<>#\"{}|\\^[]`☺\t:/@$'()*,;",
                   "+%3F%26%3D%23%2B%25%21%3C%3E%23%22%7B%7D%7C%5C%5E%5B%5D%60%"
                   "E2%98%BA%09%3A%2F%40%24%27%28%29%2A%2C%3B",
                   url_error{}}));

// Test escape_path

class MultipleEscapePathTests : public ::testing::TestWithParam<EscapeTest> {};

TEST_P(MultipleEscapePathTests, EscapePath) {
    std::string path = batteries::net::escape_path(GetParam().in);
    EXPECT_EQ(GetParam().out, path);
}

INSTANTIATE_TEST_SUITE_P(
    EscapeTests, MultipleEscapePathTests,
    ::testing::Values(
        EscapeTest{"", "", url_error{}}, EscapeTest{"abc", "abc", url_error{}},
        EscapeTest{"abc+def", "abc+def", url_error{}},
        EscapeTest{"a/b", "a%2Fb", url_error{}},
        EscapeTest{"one two", "one%20two", url_error{}},
        EscapeTest{"10%", "10%25", url_error{}},
        EscapeTest{" ?&=#+%!<>#\"{}|\\^[]`☺\t:/@$'()*,;",
                   "%20%3F&=%23+%25%21%3C%3E%23%22%7B%7D%7C%5C%5E%5B%5D%60%E2%"
                   "98%BA%09:%2F@$%27%28%29%2A%2C%3B",
                   url_error{}}));

struct ParseHostTest {
    std::string in;
    std::string host;
    std::string port;
    url_error err;
};

std::ostream& operator<<(std::ostream& os, const ParseHostTest& pht) {
    return os << "in: " << pht.in << ", host: " << pht.host
              << ", port:" << pht.port;
}

class MultipleParseHostTests : public ::testing::TestWithParam<ParseHostTest> {

};

TEST_P(MultipleParseHostTests, ParseHostTest) {
    std::string host, port;
    url_error err;
    std::tie(host, port, err) =
        batteries::net::internal::parse_host(GetParam().in);
    EXPECT_EQ(GetParam().err, err);
    EXPECT_EQ(GetParam().host, host);
    EXPECT_EQ(GetParam().port, port);
}

INSTANTIATE_TEST_SUITE_P(
    ParseHostTest, MultipleParseHostTests,
    ::testing::Values(
        ParseHostTest{"foo.com:80", "foo.com", "80", url_error{}},
        ParseHostTest{"foo.com", "foo.com", "", url_error{}},
        ParseHostTest{"foo.com:", "foo.com", "", url_error{}},
        ParseHostTest{"FOO.COM", "FOO.COM", "",
                      url_error{}}, // no canonicalization
        ParseHostTest{"1.2.3.4", "1.2.3.4", "", url_error{}},
        ParseHostTest{"1.2.3.4:80", "1.2.3.4", "80", url_error{}},
        ParseHostTest{"[1:2:3:4]", "[1:2:3:4]", "", url_error{}},
        ParseHostTest{"[1:2:3:4]:80", "[1:2:3:4]", "80", url_error{}},
        ParseHostTest{"[::1]:80", "[::1]", "80", url_error{}},
        ParseHostTest{"[::1]", "[::1]", "", url_error{}},
        ParseHostTest{"[::1]:", "[::1]", "", url_error{}},
        ParseHostTest{"[fe80::1%25en0]", "[fe80::1%en0]", "", url_error{}},
        ParseHostTest{"[fe80::1%25en0]:8080", "[fe80::1%en0]", "8080",
                      url_error{}},
        ParseHostTest{"[fe80::1%25%65%6e%301-._~]", "[fe80::1%en01-._~]", "",
                      url_error{}},
        ParseHostTest{"[fe80::1%25%65%6e%301-._~]:8080", "[fe80::1%en01-._~]",
                      "8080", url_error{}},
        ParseHostTest{"localhost", "localhost", "", url_error{}},
        ParseHostTest{"localhost:443", "localhost", "443", url_error{}},
        ParseHostTest{"some.super.long.domain.example.org:8080",
                      "some.super.long.domain.example.org", "8080"},
        ParseHostTest{"[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:17000",
                      "[2001:0db8:85a3:0000:0000:8a2e:0370:7334]", "17000"},
        ParseHostTest{"[2001:0db8:85a3:0000:0000:8a2e:0370:7334]",
                      "[2001:0db8:85a3:0000:0000:8a2e:0370:7334]", ""}));

struct ParseAuthorityTest {
    std::string in;
    std::string username;
    std::string password;
    std::string_view host;
    url_error err;
};

std::ostream& operator<<(std::ostream& os, const ParseAuthorityTest& pat) {
    return os << "in: " << pat.in << ", username: " << pat.username
              << ", password: " << pat.password << ", host: " << pat.host
              << ", err: " << pat.err;
}

class MultipleParseAuthorityTests
    : public ::testing::TestWithParam<ParseAuthorityTest> {};

TEST_P(MultipleParseAuthorityTests, ParseAuthorityTests) {
    std::string username, password;
    std::string_view host;
    url_error err;
    std::tie(username, password, host, err) =
        batteries::net::internal::parse_authority(GetParam().in);
    EXPECT_EQ(GetParam().username, username);
    EXPECT_EQ(GetParam().password, password);
    EXPECT_EQ(GetParam().host, host);
    EXPECT_EQ(GetParam().err, err);
}

INSTANTIATE_TEST_SUITE_P(
    ParseAuthorityTest, MultipleParseAuthorityTests,
    ::testing::Values(
        ParseAuthorityTest{"user:password@foo.com:80", "user", "password",
                           "foo.com:80", url_error{}},
        ParseAuthorityTest{"foo.com:80", "", "", "foo.com:80", url_error{}},
        ParseAuthorityTest{// malformed host should not cause error
                           "user@password:foo.com:80", "user", "",
                           "password:foo.com:80", url_error{}},
        ParseAuthorityTest{
            "us^er:password@foo.com:80", "", "", "",
            url_error(url_error_code::parse_error, "invalid userinfo")}));

struct ParseSchemeTest {
    std::string in;
    std::string scheme;
    std::string_view rest;
    url_error err;
};

std::ostream& operator<<(std::ostream& os, const ParseSchemeTest& pst) {
    return os << "in: " << pst.in << ", scheme: " << pst.scheme
              << ", rest: " << pst.rest << ", err: " << pst.err;
}

class MultipleParseSchemeTests
    : public ::testing::TestWithParam<ParseSchemeTest> {};

TEST_P(MultipleParseSchemeTests, ParseSchemeTests) {
    std::string scheme;
    std::string_view rest;
    url_error err;
    std::tie(scheme, rest, err) =
        batteries::net::internal::parse_scheme(GetParam().in);
    EXPECT_EQ(GetParam().scheme, scheme);
    EXPECT_EQ(GetParam().rest, rest);
    EXPECT_EQ(GetParam().err, err);
}

INSTANTIATE_TEST_SUITE_P(
    ParseSchemeTest, MultipleParseSchemeTests,
    ::testing::Values(
        ParseSchemeTest{"http://root:password@192.168.1.1:8080/path/to/file",
                        "http", "//root:password@192.168.1.1:8080/path/to/file",
                        url_error{}},
        ParseSchemeTest{"file://test.txt", "file", "//test.txt", url_error{}}));

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
    std::string raw_path;
    batteries::net::query query;
    std::string fragment;
    url_error err;
};

std::ostream& operator<<(std::ostream& os, const ParseUrlTest& pt) {
    return os << "in: " << pt.in << ", scheme: " << pt.scheme
              << ", opaque: " << pt.opaque << ", username: " << pt.username
              << ", password: " << pt.password << ", host: " << pt.host
              << ", hostname: " << pt.hostname << ", port: " << pt.port
              << ", path: " << pt.path << ", raw_path: " << pt.raw_path
              << ", query: " << pt.query.to_string()
              << ", fragment: " << pt.fragment << ", err: " << pt.err;
}

class MultipleParseUrlTests : public ::testing::TestWithParam<ParseUrlTest> {
  protected:
    batteries::net::url url;
};

TEST_P(MultipleParseUrlTests, ParseUrlTests) {
    url_error err = url.parse(GetParam().in);
    EXPECT_EQ(GetParam().err, err);
    EXPECT_EQ(GetParam().scheme, url.scheme());
    EXPECT_EQ(GetParam().opaque, url.opaque());
    EXPECT_EQ(GetParam().username, url.username());
    EXPECT_EQ(GetParam().password, url.password());
    EXPECT_EQ(GetParam().host, url.host());
    EXPECT_EQ(GetParam().hostname, url.hostname());
    EXPECT_EQ(GetParam().port, url.port());
    EXPECT_EQ(GetParam().path, url.path());
    EXPECT_EQ(GetParam().raw_path, url.raw_path());
    EXPECT_EQ(GetParam().query, url.query());
    EXPECT_EQ(GetParam().fragment, url.fragment());
}

INSTANTIATE_TEST_SUITE_P(
    ParseUrlTest, MultipleParseUrlTests,
    ::testing::Values(
        ParseUrlTest{"http://foo.com", "http", "", "", "", "foo.com", "foo.com",
                     "", "", "", batteries::net::query(), "", url_error{}},
        ParseUrlTest{"http://foo.com/", "http", "", "", "", "foo.com",
                     "foo.com", "", "/", "", batteries::net::query(), "",
                     url_error{}},
        ParseUrlTest{"http://foo.com/path", "http", "", "", "", "foo.com",
                     "foo.com", "", "/path", "", batteries::net::query(), "",
                     url_error{}},
        ParseUrlTest{"http://foo.com/path%20escaped", "http", "", "", "",
                     "foo.com", "foo.com", "", "/path escaped",
                     "/path%20escaped", batteries::net::query(), "",
                     url_error{}},
        ParseUrlTest{"http://foo.com/path?a=1&b=2", "http", "", "", "",
                     "foo.com", "foo.com", "", "/path", "",
                     batteries::net::query(batteries::net::query_values{
                         {"a", "1"}, {"b", "2"}}),
                     "", url_error{}},
        ParseUrlTest{
            "/",                     // in
            "",                      // scheme
            "",                      // opaque
            "",                      // username
            "",                      // password
            "",                      // host
            "",                      // hostname
            "",                      // port
            "/",                     // path
            "",                      // raw_path
            batteries::net::query(), // query
            "",                      // fragment
            url_error{}              // error
        },
        ParseUrlTest{
            "*",                     // in
            "",                      // scheme
            "",                      // opaque
            "",                      // username
            "",                      // password
            "",                      // host
            "",                      // hostname
            "",                      // port
            "*",                     // path
            "",                      // raw_path
            batteries::net::query(), // query
            "",                      // fragment
            url_error{}              // error
        },
        ParseUrlTest{
            "http://192.168.0.1:8080/", // in
            "http",                     // scheme
            "",                         // opaque
            "",                         // username
            "",                         // password
            "192.168.0.1:8080",         // host
            "192.168.0.1",              // hostname
            "8080",                     // port
            "/",                        // path
            "",                         // raw_path
            batteries::net::query(),    // query
            "",                         // fragment
            url_error{}                 // error
        },
        ParseUrlTest{
            "http://192.168.0.1:8080/?a=1&b=2", // in
            "http",                             // scheme
            "",                                 // opaque
            "",                                 // username
            "",                                 // password
            "192.168.0.1:8080",                 // host
            "192.168.0.1",                      // hostname
            "8080",                             // port
            "/",                                // path
            "",                                 // raw_path
            batteries::net::query(batteries::net::query_values{
                {"a", "1"}, {"b", "2"}}), // query
            "",                           // fragment
            url_error{}                   // error
        },
        ParseUrlTest{
            "http://[fe80::1]/",     // in
            "http",                  // scheme
            "",                      // opaque
            "",                      // username
            "",                      // password
            "[fe80::1]",             // host
            "[fe80::1]",             // hostname
            "",                      // port
            "/",                     // path
            "",                      // raw_path
            batteries::net::query(), // query
            "",                      // fragment
            url_error{}              // error
        },
        ParseUrlTest{
            "http://[fe80::1]:8080/", // in
            "http",                   // scheme
            "",                       // opaque
            "",                       // username
            "",                       // password
            "[fe80::1]:8080",         // host
            "[fe80::1]",              // hostname
            "8080",                   // port
            "/",                      // path
            "",                       // raw_path
            batteries::net::query(),  // query
            "",                       // fragment
            url_error{}               // error
        },
        ParseUrlTest{
            "http://[fe80::1]:8080/?a=1&b=2", // in
            "http",                           // scheme
            "",                               // opaque
            "",                               // username
            "",                               // password
            "[fe80::1]:8080",                 // host
            "[fe80::1]",                      // hostname
            "8080",                           // port
            "/",                              // path
            "",                               // raw_path
            batteries::net::query(batteries::net::query_values{
                {"a", "1"}, {"b", "2"}}), // query
            "",                           // fragment
            url_error{}                   // error
        },
        ParseUrlTest{
            "http://root:password@192.168.1.1:8080/path/to/file#nonsense",
            "http", "", "root", "password", "192.168.1.1:8080", "192.168.1.1",
            "8080", "/path/to/file", "", batteries::net::query(), "nonsense",
            url_error{}},
        ParseUrlTest{
            "http://[fe80::1%25en0]/", // in
            "http",                    // scheme
            "",                        // opaque
            "",                        // username
            "",                        // password
            "[fe80::1%en0]",           // host
            "[fe80::1%en0]",           // hostname
            "",                        // port
            "/",                       // path
            "",                        // raw_path
            batteries::net::query(),   // query
            "",                        // fragment
            url_error{}                // error
        },
        ParseUrlTest{
            "http://[fe80::1%25en0]:8080/", // in
            "http",                         // scheme
            "",                             // opaque
            "",                             // username
            "",                             // password
            "[fe80::1%en0]:8080",           // host
            "[fe80::1%en0]",                // hostname
            "8080",                         // port
            "/",                            // path
            "",                             // raw_path
            batteries::net::query(),        // query
            "",                             // fragment
            url_error{}                     // error
        },
        ParseUrlTest{
            "http://[fe80::1%25%65%6e%301-._~]/", // in
            "http",                               // scheme
            "",                                   // opaque
            "",                                   // username
            "",                                   // password
            "[fe80::1%en01-._~]",                 // host
            "[fe80::1%en01-._~]",                 // hostname
            "",                                   // port
            "/",                                  // path
            "",                                   // raw_path
            batteries::net::query(),              // query
            "",                                   // fragment
            url_error{}                           // error
        },
        ParseUrlTest{
            "http://[fe80::1%25%65%6e%301-._~]:8080/", // in
            "http",                                    // scheme
            "",                                        // opaque
            "",                                        // username
            "",                                        // password
            "[fe80::1%en01-._~]:8080",                 // host
            "[fe80::1%en01-._~]",                      // hostname
            "8080",                                    // port
            "/",                                       // path
            "",                                        // raw_path
            batteries::net::query(),                   // query
            "",                                        // fragment
            url_error{}                                // error
        },
        ParseUrlTest{
            "foo.html",              // in
            "",                      // scheme
            "foo.html",              // opaque
            "",                      // username
            "",                      // password
            "",                      // host
            "",                      // hostname
            "",                      // port
            "",                      // path
            "",                      // raw_path
            batteries::net::query(), // query
            "",                      // fragment
            url_error{}              // error
        },
        ParseUrlTest{
            "../dir/",               // in
            "",                      // scheme
            "../dir/",               // opaque
            "",                      // username
            "",                      // password
            "",                      // host
            "",                      // hostname
            "",                      // port
            "",                      // path
            "",                      // raw_path
            batteries::net::query(), // query
            "",                      // fragment
            url_error{}              // error
        },
        ParseUrlTest{
            "http://192.168.0.%31/",                       // in
            "http",                                        // scheme
            "",                                            // opaque
            "",                                            // username
            "",                                            // password
            "",                                            // host
            "",                                            // hostname
            "",                                            // port
            "",                                            // path
            "",                                            // raw_path
            batteries::net::query(),                       // query
            "",                                            // fragment
            url_error(url_error_code::escape_error, "%31") // error
        },
        ParseUrlTest{
            "http://192.168.0.%31:8080/",                  // in
            "http",                                        // scheme
            "",                                            // opaque
            "",                                            // username
            "",                                            // password
            "",                                            // host
            "",                                            // hostname
            "",                                            // port
            "",                                            // path
            "",                                            // raw_path
            batteries::net::query(),                       // query
            "",                                            // fragment
            url_error(url_error_code::escape_error, "%31") // error
        },
        ParseUrlTest{
            "http://[fe80::%31]/",                          // in
            "http",                                         // scheme
            "",                                             // opaque
            "",                                             // username
            "",                                             // password
            "",                                             // host
            "",                                             // hostname
            "",                                             // port
            "",                                             // path
            "",                                             // raw_path
            batteries::net::query(),                        // query
            "",                                             // fragment
            url_error(url_error_code::escape_error, "%31]") // error
        },
        ParseUrlTest{
            "http://[fe80::%31]:8080/",                     // in
            "http",                                         // scheme
            "",                                             // opaque
            "",                                             // username
            "",                                             // password
            "",                                             // host
            "",                                             // hostname
            "",                                             // port
            "",                                             // path
            "",                                             // raw_path
            batteries::net::query(),                        // query
            "",                                             // fragment
            url_error(url_error_code::escape_error, "%31]") // error
        },
        ParseUrlTest{
            "http://[fe80::%31%25en0]/",                   // in
            "http",                                        // scheme
            "",                                            // opaque
            "",                                            // username
            "",                                            // password
            "",                                            // host
            "",                                            // hostname
            "",                                            // port
            "",                                            // path
            "",                                            // raw_path
            batteries::net::query(),                       // query
            "",                                            // fragment
            url_error(url_error_code::escape_error, "%31") // error
        },
        ParseUrlTest{
            "http://[fe80::%31%25en0]:8080/",              // in
            "http",                                        // scheme
            "",                                            // opaque
            "",                                            // username
            "",                                            // password
            "",                                            // host
            "",                                            // hostname
            "",                                            // port
            "",                                            // path
            "",                                            // raw_path
            batteries::net::query(),                       // query
            "",                                            // fragment
            url_error(url_error_code::escape_error, "%31") // error
        },
        // These two cases are valid as textual representations as
        // described in RFC 4007, but are not valid as address
        // literals with IPv6 zone identifiers in URIs as described in
        // RFC 6874.
        ParseUrlTest{
            "http://[fe80::1%en0]/",                         // in
            "http",                                          // scheme
            "",                                              // opaque
            "",                                              // username
            "",                                              // password
            "",                                              // host
            "",                                              // hostname
            "",                                              // port
            "",                                              // path
            "",                                              // raw_path
            batteries::net::query(),                         // query
            "",                                              // fragment
            url_error(url_error_code::escape_error, "%en0]") // error
        },
        ParseUrlTest{
            "http://[fe80::1%en0]:8080//",                   // in
            "http",                                          // scheme
            "",                                              // opaque
            "",                                              // username
            "",                                              // password
            "",                                              // host
            "",                                              // hostname
            "",                                              // port
            "",                                              // path
            "",                                              // raw_path
            batteries::net::query(),                         // query
            "",                                              // fragment
            url_error(url_error_code::escape_error, "%en0]") // error
        }));

using RoundTripTest = std::string;

class MultipleRoundTripTests : public ::testing::TestWithParam<RoundTripTest> {
  protected:
    batteries::net::url url;
};

TEST_P(MultipleRoundTripTests, RoundTripTests) {
    url_error err = url.parse(GetParam());
    EXPECT_EQ(err, url_error{});
    EXPECT_EQ(GetParam(), url.to_string());
}

INSTANTIATE_TEST_SUITE_P(
    RoundTripTest, MultipleRoundTripTests,
    ::testing::Values(
        RoundTripTest{"http://foo.com"}, RoundTripTest{"http://foo.com/"},
        RoundTripTest{"http://foo.com/path/to/file"},
        RoundTripTest{"http://foo.com/path%20escaped"},
        RoundTripTest{"http://foo.com/path?a=1&b=2"},
        RoundTripTest{
            "/", // in
        },
        RoundTripTest{
            "*", // in
        },
        RoundTripTest{"http://192.168.0.1:8080/"},
        RoundTripTest{"http://192.168.0.1:8080/?a=1&b=2"},
        RoundTripTest{"http://[fe80::1]/"},
        RoundTripTest{"http://[fe80::1]:8080/"},
        RoundTripTest{"http://[fe80::1]:8080/?a=1&b=2"},
        RoundTripTest{
            "http://root:password@192.168.1.1:8080/path/to/file#nonsense"},
        RoundTripTest{"http://[fe80::1%25en0]/"},
        RoundTripTest{"http://[fe80::1%25en0]:8080/"}));

TEST(InitTest, InitTest) {
    std::string urlString =
        "http://root:password@192.168.1.1:8080/path/to/file#nonsense";
    batteries::net::url url1;
    batteries::net::url url2(urlString);
    url1.parse(urlString);

    EXPECT_EQ(url1, url2);
}

} // namespace
