# Batteries
A lot of languages claim to be "batteries included" meaning they have a lot of support libraries to keep you from having to roll your own code.
Batteries aims to fill some of that gap for C++. There are many utilities here that are I have found useful. Google's Abseil project is a
dependency because I don't want to have to duplicate perfectly good code.

## Requirements
1. Conan is used for package management. You can install the most recent version at https://conan.io. If someone would like to recommend an
alternate way of managing this dependency I would appreciate some help.
2. Abseil is a requirement however the current master branch must be used. Unfortunately the string_view::at function was not included until
after the last LTS. Until the next the next LTS release you'll need to build your own version:
```
git clone https://github.com/abseil/abseil-cpp.git
cd abseil-cpp
conan create . abseil/master@bincrafters/stable --keep-source
```

## Building
To build this you will need CMake version 3.9 or higher. Conan can also be used to build the library.

```
conan create . batteries/master@bincrafters/stable --keep-source
```
NOTE: This is not a bincrafters supported package.

## Using in Your Project
To use this project you will need to add the following into your CMakeLists.txt file:
```
# integrate conan build process
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
```

To install the packages run `conan install . --build missing`