# Batteries
A lot of languages claim to be "batteries included" meaning they have a lot of support libraries to keep you from having to roll your own code.
Batteries aims to fill some of that gap for C++. There are many utilities here that are I have found useful. Google's Abseil project is a
dependency because I don't want to have to duplicate perfectly good code.

## Requirements
1. Conan is optionally used for package management. You can install the most recent version at https://conan.io. If you have python it can be installed by
`pip install conan`
2. Abseil is a requirement for string parsing. If you're using conan you can install this dependency using:
```bash
mkdir build
cd build
conan install ..
```

## Building
To build this you will need CMake version 3.15 or higher. Conan can also be used to build the library.

```
conan create . batteries/main@bincrafters/stable --keep-source
```
NOTE: This is not a bincrafters supported package.

## Using in Your Project
To use this project you will need to add the following into your CMakeLists.txt file:
```cmake
# integrate conan build process
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
```

