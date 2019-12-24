#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
    exports_sources = ["CMakeLists.txt", "CMake/*", "batteries/*"]
    generators = "cmake"
    settings = "os", "arch", "compiler", "build_type"
    requires = "abseil/master@bincrafters/stable"

    def configure(self):
        if self.settings.os == "Windows" and \
           self.settings.compiler == "Visual Studio" and \
           Version(self.settings.compiler.version.value) < "14":
            raise ConanInvalidConfiguration("Batteries does not support MSVC < 14")

    def set_version(self):
        git = tools.Git(folder=self.recipe_folder)
        self.version = "%s_%s" % (git.get_branch(), git.get_revision())

    def build(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_TESTING"] = False
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("LICENSE", dst="licenses")
        self.copy("*.h", dst="include", src=".")
        self.copy("*.inc", dst="include", src=".")
        self.copy("*.a", dst="lib", src=".", keep_path=False)
        self.copy("*.lib", dst="lib", src=".", keep_path=False)

    def package_info(self):
        if self.settings.os == "Linux":
            self.cpp_info.libs = ["-Wl,--start-group"]
        self.cpp_info.libs.extend(tools.collect_libs(self))
        if self.settings.os == "Linux":
            self.cpp_info.libs.extend(["-Wl,--end-group", "pthread"])