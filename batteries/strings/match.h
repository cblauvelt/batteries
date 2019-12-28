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

#pragma once

#include <absl/strings/ascii.h>
#include <absl/strings/string_view.h>

namespace batteries {

namespace strings {

unsigned int count(absl::string_view s, absl::string_view searchTerm) {
	unsigned int charCount = 0;
	auto startSearch = s.find(searchTerm, 0);
	while(startSearch != s.npos) {
		charCount++;

		// Handle the last character being the search term.
		if(startSearch + searchTerm.length() >= s.length()) {
			break;
		}
		startSearch = s.find(searchTerm, startSearch + searchTerm.length());
	}

	return charCount;
}

bool containsCtlByte(absl::string_view rawurl) {
	for(auto c : rawurl) {
		if(absl::ascii_iscntrl(c)) { return true; }
	}

	return false;
}

}

}