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

struct EscapeTest {
    std::string in;
    std::string out;
    batteries::net::UrlError error;
};

std::ostream & operator<<(std::ostream &os, const EscapeTest& et)
{
    return os << "in: " << et.in << ", out: " << et.out << ", error:" << et.error.message();
}

class MultipleUnescapeTests : public ::testing::TestWithParam<EscapeTest>{

};

TEST_P(MultipleUnescapeTests, Unescape) {
    std::string path;
    batteries::net::UrlError err;

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
            batteries::net::Url::UrlRangeError("%")
        },
        EscapeTest{
            "%a",
            "",
            batteries::net::Url::UrlRangeError("%a")
        },
        EscapeTest{
            "%1",
            "",
            batteries::net::Url::UrlRangeError("%1")
        },
        EscapeTest{
            "123%45%6",
            "",
            batteries::net::Url::UrlRangeError("123%45%6")
        },
        EscapeTest{
            "%zzzzz",
            "",
            batteries::net::Url::UrlEscapeError("%zz")
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

// struct EncodQueryTest{
//     batteries::net::QueryValues vales;
//     std::string expected;
// };
// class MultipleEncodeQueryTests : public ::testing::TestWithParam<EncodQueryTest>{

// };

// TEST_P(MultipleEncodeQueryTests, EscapeQuery) {
//     std::string path = batteries::net::escapeQuery(GetParam().in);
//     EXPECT_EQ(GetParam().out, path);
// }

// INSTANTIATE_TEST_CASE_P(
//     EscapeTests,
//     MultipleEncodeQueryTests,
//     ::testing::Values(
//         EscapeTest{
//             "",
//             "",
//             batteries::net::UrlNoError
//         },
//         EscapeTest{
//             "abc",
//             "abc",
//             batteries::net::UrlNoError
//         },
//         EscapeTest{
//             "one two",
// 	    	"one+two",
//             batteries::net::UrlNoError
//         },
//         EscapeTest{
//             "10%",
// 		    "10%25",
//             batteries::net::UrlNoError
//         },
//         EscapeTest{
//             " ?&=#+%!<>#\"{}|\\^[]`☺\t:/@$'()*,;",
// 		    "+%3F%26%3D%23%2B%25%21%3C%3E%23%22%7B%7D%7C%5C%5E%5B%5D%60%E2%98%BA%09%3A%2F%40%24%27%28%29%2A%2C%3B",
//             batteries::net::UrlNoError
//         }
//     )
// );

} // namespace