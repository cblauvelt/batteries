#
# Copyright 2019 The Batteries Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#			https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

batteries_cc_library(
	NAME
		net
	HDRS
		"base.hpp"
		"internal/parse.hpp"
		"internal/escape.hpp"
		"url.hpp"
		"query.hpp"
	SRCS
		"base.cpp"
		"internal/parse.cpp"
		"internal/escape.cpp"
		"url.cpp"
		"query.cpp"
	COPTS
		${BATT_DEFAULT_COPTS}
	DEPS
		${CONAN_LIBS}
	PUBLIC
)

batteries_cc_test(
	NAME
		net_test
	SRCS
		"url_test.cpp"
		"query_test.cpp"
	COPTS
		${BATT_TEST_COPTS}
	DEPS
		batteries::net
		${CONAN_LIBS}
)
