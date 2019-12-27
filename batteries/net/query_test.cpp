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

#include "query.h"

#include <absl/strings/str_replace.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using QueryMap = batteries::net::internal::QueryMap;
using UrlError = batteries::net::UrlError;


// Test parseQuery

struct ParseQueryTest{
    std::string query;
    QueryMap map;
    UrlError err;
};
class MultipleParseQueryTests : public ::testing::TestWithParam<ParseQueryTest>{

};

TEST_P(MultipleParseQueryTests, EscapeQuery) {
    QueryMap map;
    UrlError err;
    std::tie(map, err) = batteries::net::internal::parseQuery(GetParam().query);
    EXPECT_EQ(GetParam().err, err);
    EXPECT_EQ(GetParam().map, map);
}

INSTANTIATE_TEST_CASE_P(
    QueryTests,
    MultipleParseQueryTests,
    ::testing::Values(
        ParseQueryTest{
            "a=1&b=2",
            QueryMap{{"a", "1"}, {"b", "2"}},
            batteries::net::UrlNoError
        },ParseQueryTest{
            "a=1&a=2&a=banana",
            QueryMap{{"a", "1"}, {"a", "2"}, {"a", "banana"}},
            batteries::net::UrlNoError
        },ParseQueryTest{
            "ascii=%3Ckey%3A+0x90%3E",
            QueryMap{{"ascii", "<key: 0x90>"}},
            batteries::net::UrlNoError
        },ParseQueryTest{
            "a=1;b=2",
            QueryMap{{"a", "1"}, {"b", "2"}},
            batteries::net::UrlNoError
        },ParseQueryTest{
            "a=1&a=2;a=banana",
            QueryMap{{"a", "1"}, {"a", "2"}, {"a", "banana"}},
            batteries::net::UrlNoError
        },ParseQueryTest{
            "a=1&&a=2;a=banana",
            QueryMap{},
            batteries::net::UrlParseError("a=1&&a=2;a=banana")
        },ParseQueryTest{
            "a=1&a=2;;a=banana",
            QueryMap{},
            batteries::net::UrlParseError("a=1&a=2;;a=banana")
        },ParseQueryTest{
            "a=1&a=2;a==banana",
            QueryMap{},
            batteries::net::UrlParseError("a=1&a=2;a==banana")
        },ParseQueryTest{
            "a==1&a=2;a=banana",
            QueryMap{},
            batteries::net::UrlParseError("a==1&a=2;a=banana")
        }
    )
);

// Test parseValues

struct BuildQueryTest{
    std::string query;
    QueryMap map;
};
class MultipleBuildQueryTests : public ::testing::TestWithParam<BuildQueryTest>{

};

TEST_P(MultipleBuildQueryTests, EscapeQuery) {
    std::string query = batteries::net::internal::buildQuery(GetParam().map.cbegin(), GetParam().map.cend());
    EXPECT_EQ(GetParam().query, query);
}

INSTANTIATE_TEST_CASE_P(
    QueryTests,
    MultipleBuildQueryTests,
    ::testing::Values(
        BuildQueryTest{
            "a=1&b=2",
            QueryMap{{"a", "1"}, {"b", "2"}}
        },BuildQueryTest{
            "a=1&a=2&a=banana",
            QueryMap{{"a", "1"}, {"a", "2"}, {"a", "banana"}}
        },BuildQueryTest{
            "ascii=%3Ckey%3A+0x90%3E",
            QueryMap{{"ascii", "<key: 0x90>"}}
        },BuildQueryTest{
            "a=1&a=2&a=banana&ba=1&ba=2&ba=banana",
            QueryMap{{"a", "1"}, {"a", "2"}, {"a", "banana"}, {"ba", "1"}, {"ba", "2"}, {"ba", "banana"}}
        }
    )
);

}