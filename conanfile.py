#!/usr/bin/env python
# Copyright 2019 The Batteries Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# -*- coding: utf-8 -*-

import re
from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
from conans.model.version import Version


class BatteriesConan(ConanFile):
    name = "batteries"
    url = "https://github.com/cblauvelt/batteries"
    homepage = url
    author = "Batteries Authors"
    description = "Batteries Included Libraries"
    license = "Apache-2.0"
    topics = ("conan", "batteries", "batteries-cpp", "common-libraries")
    exports = ["LICENSE"]
    exports_sources = ["CMakeLists.txt", "conan.cmake",
                       "conanfile.py", "CMake/*", "batteries/*"]
    generators = "cmake"
    settings = "os", "arch", "compiler", "build_type"
    requires = "abseil/20211102.0"
    build_requires = "gtest/cci.20210126"
    options = {"cxx_standard": [17, 20], "build_testing": [
        True, False], "fPIC": [True, False]}
    default_options = {"cxx_standard": 20, "build_testing": True, "fPIC": True}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.settings.os == "Windows" and \
           self.settings.compiler == "Visual Studio" and \
           Version(self.settings.compiler.version.value) < "14":
            raise ConanInvalidConfiguration(
                "Batteries does not support MSVC < 14")

    def sanitize_tag(self, version):
        return re.sub(r'^v', '', version)

    def sanitize_branch(self, branch):
        return re.sub(r'/', '_', branch)

    def set_version(self):
        git = tools.Git(folder=self.recipe_folder)
        self.version = self.sanitize_tag(git.get_tag()) if git.get_tag(
        ) else "%s_%s" % (self.sanitize_branch(git.get_branch()), git.get_revision()[:12])

    def build(self):
        cmake = CMake(self)
        cmake.definitions["BATT_RUN_TESTS"] = self.options.build_testing
        cmake.definitions["BUILD_TESTING"] = self.options.build_testing
        cmake.definitions["CMAKE_CXX_STANDARD"] = self.options.cxx_standard
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        self.copy("LICENSE", dst="licenses")
        self.copy("*.hpp", dst="include", src=".")
        self.copy("*.inc", dst="include", src=".")
        self.copy("*.a", dst="lib", src=".", keep_path=False)
        self.copy("*.lib", dst="lib", src=".", keep_path=False)

    def package_info(self):
        if self.settings.os == "Linux":
            self.cpp_info.libs = ["-Wl,--start-group"]
        self.cpp_info.libs.extend(tools.collect_libs(self))
        if self.settings.os == "Linux":
            self.cpp_info.libs.extend(["-Wl,--end-group", "pthread"])
