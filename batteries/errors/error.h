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

#pragma once

#include <string>
#include <ostream>

namespace batteries {

namespace errors {

enum class GenericErrorCode {
    NoError = 0,
    GenericError
};

template <typename T = GenericErrorCode>
class Error {

public:
    Error();
    Error(T errorCode, std::string message);
    Error(std::string message);

    T errorCode() const;

    std::string message() const;
    std::string what() const;

    bool operator==(const Error<T> &rhs) const;
    bool operator!=(const Error<T> &rhs) const;
    bool operator!() const;

    explicit operator bool() const {
        return (mErrorCode != static_cast<T>(0));
    }

    friend std::ostream& operator<< (std::ostream &os, const Error<T>& error) {
        return os << "error_code: " << (int)error.mErrorCode << ", message: " << error.mMessage;
    }

private:
    T mErrorCode;
    std::string mMessage;
};

static const Error<> NoError;

template <typename T>
Error<T>::Error() :
    mErrorCode(static_cast<T>(0)),
    mMessage()
{}

template <typename T>
Error<T>::Error(T errorCode, std::string message) :
    mErrorCode(errorCode),
    mMessage(message)
{}

template <typename T>
T Error<T>::errorCode() const {
    return mErrorCode;
}

template <typename T>
std::string Error<T>::message() const {
    return mMessage;
}

template <typename T>
std::string Error<T>::what() const {
    return message();
}

template <typename T>
bool Error<T>::operator==(const Error<T> &rhs) const {
    return (
        mErrorCode == rhs.mErrorCode &&
        mMessage == rhs.mMessage
    );
}

template <typename T>
bool Error<T>::operator!=(const Error<T> &rhs) const {
    return !(*this == rhs);
}
    

template <typename T>
bool Error<T>::operator!() const {
    return !((bool)*this);
}
    

}

}