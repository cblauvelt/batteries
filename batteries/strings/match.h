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

/**
 * @brief count counts the number of times searchTerm appears in s.
 * @param s A string that is to be searched.
 * @param searchTerm The term that will be searched for in s.
 * @returns The number of times searchTerm appears in s.
 */
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

/**
 * @brief containsCtlChar searched candidate for a control character.
 * @param candidate The string to search for control characters.
 * @returns true if candidate contains a control character, otherwise false.
 */
bool containsCtlChar(absl::string_view candidate) {
	for(auto c : candidate) {
		if(absl::ascii_iscntrl(c)) { return true; }
	}

	return false;
}

}

}