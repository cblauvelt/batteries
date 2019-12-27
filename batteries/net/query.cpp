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

UrlError Query::parse(std::string query) {
	mRawQuery = std::move(query);
	mQuery.clear();
	UrlError err;
	std::tie(mQuery, err)= internal::parseQuery(mRawQuery);
	if(err != UrlNoError) {
		return err;
	}
}

std::string Query::toString() {
	if(mRawQueryDirty) { // Rebuild raw query
		mRawQuery = internal::buildQuery(mQuery.cbegin(), mQuery.cend());
		mRawQueryDirty = false;
	}
	return mRawQuery;
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

}

}