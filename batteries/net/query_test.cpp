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

#include "batteries/errors/error.hpp"
#include "query.hpp"

#include <absl/strings/str_replace.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using query_map = batteries::net::internal::query_map;
using batteries::errors::no_error;
using batteries::net::error;

// Test parse_query

struct ParseQueryTest {
    std::string query;
    query_map map;
    error err;
};
class MultipleParseQueryTests
    : public ::testing::TestWithParam<ParseQueryTest> {};

TEST_P(MultipleParseQueryTests, EscapeQuery) {
    query_map map;
    error err;
    std::tie(map, err) =
        batteries::net::internal::parse_query(GetParam().query);
    EXPECT_EQ(GetParam().err, err);
    EXPECT_EQ(GetParam().map, map);
}

INSTANTIATE_TEST_SUITE_P(
    QueryTests, MultipleParseQueryTests,
    ::testing::Values(
        ParseQueryTest{"a=1&b=2", query_map{{"a", "1"}, {"b", "2"}}, no_error},
        ParseQueryTest{"a=1&a=2&a=banana",
                       query_map{{"a", "1"}, {"a", "2"}, {"a", "banana"}},
                       no_error},
        ParseQueryTest{"ascii=%3Ckey%3A+0x90%3E",
                       query_map{{"ascii", "<key: 0x90>"}}, no_error},
        ParseQueryTest{"a=1;b=2", query_map{{"a", "1"}, {"b", "2"}}, no_error},
        ParseQueryTest{"a=1&a=2;a=banana",
                       query_map{{"a", "1"}, {"a", "2"}, {"a", "banana"}},
                       no_error},
        ParseQueryTest{
            "a=1&&a=2;a=banana", query_map{},
            batteries::net::error(batteries::net::url_error_code::parse_error,
                                  "a=1&&a=2;a=banana")},
        ParseQueryTest{
            "a=1&a=2;;a=banana", query_map{},
            batteries::net::error(batteries::net::url_error_code::parse_error,
                                  "a=1&a=2;;a=banana")},
        ParseQueryTest{
            "a=1&a=2;a==banana", query_map{},
            batteries::net::error(batteries::net::url_error_code::parse_error,
                                  "a=1&a=2;a==banana")},
        ParseQueryTest{
            "a==1&a=2;a=banana", query_map{},
            batteries::net::error(batteries::net::url_error_code::parse_error,
                                  "a==1&a=2;a=banana")}));

// Test get string

struct BuildQueryTest {
    std::string query;
    query_map map;
};

class MultipleBuildQueryTests
    : public ::testing::TestWithParam<BuildQueryTest> {};

TEST_P(MultipleBuildQueryTests, EscapeQuery) {
    std::string query = batteries::net::internal::build_query(
        GetParam().map.cbegin(), GetParam().map.cend());
    EXPECT_EQ(GetParam().query, query);
}

INSTANTIATE_TEST_SUITE_P(
    QueryTests, MultipleBuildQueryTests,
    ::testing::Values(
        BuildQueryTest{"a=1&b=2", query_map{{"a", "1"}, {"b", "2"}}},
        BuildQueryTest{"a=1&a=2&a=banana",
                       query_map{{"a", "1"}, {"a", "2"}, {"a", "banana"}}},
        BuildQueryTest{"ascii=%3Ckey%3A+0x90%3E",
                       query_map{{"ascii", "<key: 0x90>"}}},
        BuildQueryTest{"a=1&a=2&a=banana&ba=1&ba=2&ba=banana",
                       query_map{{"a", "1"},
                                 {"a", "2"},
                                 {"a", "banana"},
                                 {"ba", "1"},
                                 {"ba", "2"},
                                 {"ba", "banana"}}}));

// Test parseValues
struct InitialValuesTest {
    std::string query;
    batteries::net::query_values values;
};

class MultipleInitialValuesTests
    : public ::testing::TestWithParam<InitialValuesTest> {};

TEST_P(MultipleInitialValuesTests, EscapeQuery) {
    batteries::net::query query(GetParam().values);
    EXPECT_EQ(GetParam().query, query.to_string());
}

INSTANTIATE_TEST_SUITE_P(
    QueryTests, MultipleInitialValuesTests,
    ::testing::Values(
        InitialValuesTest{"?a=1&b=2",
                          batteries::net::query_values{{"a", "1"}, {"b", "2"}}},
        InitialValuesTest{"?a=1&a=2&a=banana",
                          batteries::net::query_values{
                              {"a", "1"}, {"a", "2"}, {"a", "banana"}}},
        InitialValuesTest{
            "?ascii=%3Ckey%3A+0x90%3E",
            batteries::net::query_values{{"ascii", "<key: 0x90>"}}},
        InitialValuesTest{"?a=1&a=2&a=banana&ba=1&ba=2&ba=banana",
                          batteries::net::query_values{{"a", "1"},
                                                       {"a", "2"},
                                                       {"a", "banana"},
                                                       {"ba", "1"},
                                                       {"ba", "2"},
                                                       {"ba", "banana"}}}));

} // namespace