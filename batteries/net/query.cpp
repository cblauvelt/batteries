// Copyright 2019 The Batteries Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//	  https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "query.hpp"

#include <algorithm>

#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>

namespace batteries {

namespace net {

query::query()
    : query_()
    , raw_query_()
    , force_query_(false)
    , raw_query_dirty_(false) {}

query::query(std::string query)
    : query_()
    , raw_query_()
    , force_query_(false)
    , raw_query_dirty_(false) {
    parse(query);
}

query::query(const query_values& values)
    : query_()
    , raw_query_()
    , force_query_(false)
    , raw_query_dirty_(false) {
    for (auto& value : values) {
        query_.emplace(value);
    }
    raw_query_ = internal::build_query(values.cbegin(), values.cend());
}

error query::parse(std::string query) {
    raw_query_ = std::move(query);
    query_.clear();
    error err;
    std::tie(query_, err) = internal::parse_query(raw_query_);

    return err;
}

std::string query::to_string() const {
    if (raw_query_dirty_) { // Rebuild raw query
        const_cast<query*>(this)->raw_query_ =
            internal::build_query(query_.cbegin(), query_.cend());
        const_cast<query*>(this)->raw_query_dirty_ = false;
    }

    if (raw_query_.empty()) {
        // if forced return the question mark
        if (force_query_) {
            return "?";
        }
        // return empty string
        return raw_query_;
    }

    // Return with question mark
    return absl::StrCat("?", raw_query_);
}

void query::set_force_query(bool force) { force_query_ = force; }

bool query::force_query() const { return force_query_; }

void query::set(std::string key, std::string value) {
    raw_query_dirty_ = true;
    query_.erase(key);
    query_.insert(std::make_pair(key, value));
}

void query::add(query_value value) {
    raw_query_dirty_ = true;
    query_.insert(value);
}

void query::add(std::string key, std::string value) {
    raw_query_dirty_ = true;
    query_.insert(std::make_pair(key, value));
}

void query::del(std::string key) {
    raw_query_dirty_ = true;
    query_.erase(key);
}

query_values query::values() const {
    query_values values;
    for (auto& elem : query_) {
        values.push_back(elem);
    }
    return values;
}

query_values query::get(std::string key) const {
    query_values values;
    auto it = query_.find(key);
    while (it != query_.end()) {
        values.push_back(*it++);
    }

    return values;
}

bool query::empty() const { return query_.empty(); }

std::size_t query::size() const { return query_.size(); }

bool query::operator==(const query& rhs) const {
    return (query_ == rhs.query_);
}

bool query::operator!=(const query& rhs) const { return !(*this == rhs); }

} // namespace net

} // namespace batteries