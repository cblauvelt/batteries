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

#include "match.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

TEST(Strings, Contains) {
    std::string testString =
        "Why? Why does this string contain two question marks?";
    EXPECT_EQ(0, batteries::strings::count(testString, "x"));
    EXPECT_EQ(2, batteries::strings::count(testString, "?"));
    EXPECT_EQ(2, batteries::strings::count(testString, "Why"));
}

TEST(Strings, ContainsControlChar) {
    std::string testString =
        "Why is this text so far to the right? \t\t\t\t\t text";
    EXPECT_TRUE(batteries::strings::containsCtlChar(testString));
    testString = "No control chars";
    EXPECT_FALSE(batteries::strings::containsCtlChar(testString));
}

} // namespace