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
    batteries::errors::Error<> error;
    batteries::errors::Error<> errorWithMessage(
        batteries::errors::GenericErrorCode::GenericError, errorMessage);

    EXPECT_EQ(error, batteries::errors::NoError);
    EXPECT_FALSE(error);
    EXPECT_TRUE(!error);

    EXPECT_EQ(errorWithMessage.message(), errorWithMessage.what());
    EXPECT_EQ(errorWithMessage.message(), errorMessage);
    EXPECT_EQ(errorWithMessage.errorCode(),
              batteries::errors::GenericErrorCode::GenericError);
    EXPECT_TRUE(errorWithMessage);
    EXPECT_FALSE(!errorWithMessage);
}

TEST(Errors, CustomError) {
    enum class CustomErrorCode { NoError = 0, BadError, ThisIsUnneccesary };
    std::string errorMessage = "Test Message";
    batteries::errors::Error<CustomErrorCode> error;
    batteries::errors::Error<CustomErrorCode> errorWithMessage(
        CustomErrorCode::BadError, errorMessage);

    EXPECT_EQ(error.errorCode(), CustomErrorCode::NoError);
    EXPECT_FALSE(error);
    EXPECT_TRUE(!error);

    EXPECT_EQ(errorWithMessage.message(), errorWithMessage.what());
    EXPECT_EQ(errorWithMessage.message(), errorMessage);
    EXPECT_EQ(errorWithMessage.errorCode(), CustomErrorCode::BadError);
    EXPECT_TRUE(errorWithMessage);
    EXPECT_FALSE(!errorWithMessage);
}

} // namespace