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

#include <map>
#include <string>
#include <vector>

#include <absl/strings/string_view.h>

#include "base.hpp"
#include "batteries/errors/error.hpp"
#include "internal/escape.hpp"
#include "internal/parse.hpp"

namespace batteries {

namespace net {

class query {

  public:
    query();
    query(std::string query);
    query(const query_values& values);

    /**
     * @brief parse parses the URL-encoded query string and returns
     * a map listing the values specified for each key.
     * Parsequery always returns a non-nil map containing all the
     * valid query parameters found; err describes the first decoding error
     * encountered, if any.
     *
     * query is expected to be a list of key=value settings separated by
     * ampersands or semicolons. A setting without an equals sign is
     * interpreted as a key set to an empty value.
     *
     * @param query The raw query string to parse.
     */
    error parse(std::string query);

    /**
     * @brief to_string returns the query in raw string form.
     * @returns The query in raw string form.
     */
    std::string to_string() const;

    /**
     * @brief set_force_query Allows the user to force a null query when the
     * query values are empty.
     * @param force If true forces a query, otherwise the query will not be
     * forced.
     */
    void set_force_query(bool force);

    /**
     * @brief An empty query string e.g. "?" can be forced when query values
     * are empty.
     * @returns true if a query will be forced, otherwise false.
     */
    bool force_query() const;

    // query functions
    void set(std::string key, std::string value);
    void add(query_value value);
    void add(std::string key, std::string value);
    void del(std::string key);
    query_values values() const;
    query_values get(std::string key) const;

    bool empty() const;
    std::size_t size() const;

    // Operators
    bool operator==(const query& rhs) const;
    bool operator!=(const query& rhs) const;

  private:
    std::multimap<std::string, std::string> query_;
    std::string raw_query_;
    bool force_query_;
    bool raw_query_dirty_;
};

} // namespace net

} // namespace batteries