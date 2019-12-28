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

#include "query.h"

#include <algorithm>

#include <absl/strings/str_split.h>
#include <absl/strings/str_cat.h>

namespace batteries {

namespace net {

Query::Query() :
	mQuery(),
	mRawQuery(),
	mForceQuery(false),
	mRawQueryDirty(false)
{}

Query::Query(std::string query) :
	mQuery(),
	mRawQuery(),
	mForceQuery(false),
	mRawQueryDirty(false)
{
	parse(query);
}

Query::Query(const QueryValues& values) :
	mQuery(),
	mRawQuery(),
	mForceQuery(false),
	mRawQueryDirty(false)
{
	for(auto& value : values) {
		mQuery.emplace(value);
	}
	mRawQuery = internal::buildQuery(values.cbegin(), values.cend());
}

UrlError Query::parse(std::string query) {
	mRawQuery = std::move(query);
	mQuery.clear();
	UrlError err;
	std::tie(mQuery, err)= internal::parseQuery(mRawQuery);
	if(err != UrlNoError) {
		return err;
	}
}

std::string Query::toString() const {
	if(mRawQueryDirty) { // Rebuild raw query
		const_cast<Query*>(this)->mRawQuery = internal::buildQuery(mQuery.cbegin(), mQuery.cend());
		const_cast<Query*>(this)->mRawQueryDirty = false;
	}

	if(mRawQuery.empty() ) {
		// if forced return the question mark
		if(mForceQuery) { return "?"; }
		// return empty string
		return mRawQuery;
	}
	
	// Return with question mark
	return absl::StrCat("?", mRawQuery);
}

void Query::setForceQuery(bool force) {
	mForceQuery = force;
}

bool Query::forceQuery() const {
	return mForceQuery;
}

void Query::set(std::string key, std::string value) {
	mRawQueryDirty = true;
	mQuery.erase(key);
	mQuery.insert(std::make_pair(key, value));
}

void Query::add(QueryValue value) {
	mRawQueryDirty = true;
	mQuery.insert(value);
}

void Query::add(std::string key, std::string value) {
	mRawQueryDirty = true;
	mQuery.insert(std::make_pair(key, value));
}

void Query::del(std::string key) {
	mRawQueryDirty = true;
	mQuery.erase(key);
}

QueryValues Query::values() const {
	QueryValues values;
	for(auto &elem : mQuery) {
		values.push_back(elem);
	}
	return values;
}

QueryValues Query::get(std::string key) const {
	QueryValues values;
	auto it = mQuery.find(key);
	while(it != mQuery.end()) {
		values.push_back(*it++);
	}
}

bool Query::empty() const {
	return mQuery.empty();
}

std::size_t Query::size() const {
	return mQuery.size();
}

bool Query::operator==(const Query& rhs) const {
	return (mQuery == rhs.mQuery);
}

bool Query::operator!=(const Query& rhs) const {
	return !(*this == rhs);
}

}

}