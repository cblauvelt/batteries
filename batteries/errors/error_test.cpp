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

#include "error.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

TEST(Errors, DefaultError) {
    std::string errorMessage = "Test Message";
    batteries::errors::error error;
    batteries::errors::error errorWithMessage(errorMessage);

    EXPECT_EQ(error, batteries::errors::no_error);
    EXPECT_FALSE(error);
    EXPECT_TRUE(!error);

    EXPECT_EQ(errorWithMessage.message(), errorWithMessage.what());
    EXPECT_EQ(errorWithMessage.message(),
              std::string("Generic error type: ") + errorMessage);
    EXPECT_EQ(errorWithMessage.value(),
              (int)batteries::errors::generic_error_code::generic_error);
    EXPECT_TRUE(errorWithMessage);
    EXPECT_FALSE(!errorWithMessage);
}

TEST(Errors, CustomError) {
    std::string errorMessage = "Test Message";
    batteries::errors::error error;
    batteries::errors::error errorWithMessage(
        batteries::errors::generic_error_code::generic_error, errorMessage);

    EXPECT_EQ(error.value(),
              (int)batteries::errors::generic_error_code::no_error);
    EXPECT_FALSE(error);
    EXPECT_TRUE(!error);

    EXPECT_EQ(errorWithMessage.message(), errorWithMessage.what());
    EXPECT_EQ(errorWithMessage.message(),
              std::string("Generic error type: ") + errorMessage);
    EXPECT_EQ(errorWithMessage.value(),
              (int)batteries::errors::generic_error_code::generic_error);
    EXPECT_TRUE(errorWithMessage);
    EXPECT_FALSE(!errorWithMessage);
}

} // namespace