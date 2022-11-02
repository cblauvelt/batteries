#
# Copyright 2019 The Batteries Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Required for Testing
if(BATT_BUILD_TESTING)
  find_package(GTest REQUIRED)
endif()

# Required Dependency
find_package(Threads REQUIRED)
find_package(absl REQUIRED)

# Optional Dependency, doesn't trigger error if missing
# find_package(OpenSSL)