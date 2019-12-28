# See batteries/copts/copts.py and batteries/copts/generate_copts.py
include(GENERATED_BatteriesCopts)

set(BATT_LSAN_LINKOPTS "")
set(BATT_HAVE_LSAN OFF)
set(BATT_DEFAULT_LINKOPTS "")

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
  if (MSVC)
    set(BATT_RANDOM_RANDEN_COPTS "${BATT_RANDOM_HWAES_MSVC_X64_FLAGS}")
  else()
    set(BATT_RANDOM_RANDEN_COPTS "${BATT_RANDOM_HWAES_X64_FLAGS}")
  endif()
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm")
  if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(BATT_RANDOM_RANDEN_COPTS "${BATT_RANDOM_HWAES_ARM64_FLAGS}")
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(BATT_RANDOM_RANDEN_COPTS "${BATT_RANDOM_HWAES_ARM32_FLAGS}")
  else()
    message(WARNING "Value of CMAKE_SIZEOF_VOID_P (${CMAKE_SIZEOF_VOID_P}) is not supported.")
  endif()
else()
  message(WARNING "Value of CMAKE_SYSTEM_PROCESSOR (${CMAKE_SYSTEM_PROCESSOR}) is unknown and cannot be used to set BATT_RANDOM_RANDEN_COPTS")
  set(BATT_RANDOM_RANDEN_COPTS "")
endif()


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(BATT_DEFAULT_COPTS "${BATT_GCC_FLAGS}")
  set(BATT_TEST_COPTS "${BATT_GCC_FLAGS};${BATT_GCC_TEST_FLAGS}")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  # MATCHES so we get both Clang and AppleClang
  if(MSVC)
    # clang-cl is half MSVC, half LLVM
    set(BATT_DEFAULT_COPTS "${BATT_CLANG_CL_FLAGS}")
    set(BATT_TEST_COPTS "${BATT_CLANG_CL_FLAGS};${BATT_CLANG_CL_TEST_FLAGS}")
    set(BATT_DEFAULT_LINKOPTS "${BATT_MSVC_LINKOPTS}")
  else()
    set(BATT_DEFAULT_COPTS "${BATT_LLVM_FLAGS}")
    set(BATT_TEST_COPTS "${BATT_LLVM_FLAGS};${BATT_LLVM_TEST_FLAGS}")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      # AppleClang doesn't have lsan
      # https://developer.apple.com/documentation/code_diagnostics
      if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5)
        set(BATT_LSAN_LINKOPTS "-fsanitize=leak")
        set(BATT_HAVE_LSAN ON)
      endif()
    endif()
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(BATT_DEFAULT_COPTS "${BATT_MSVC_FLAGS}")
  set(BATT_TEST_COPTS "${BATT_MSVC_FLAGS};${BATT_MSVC_TEST_FLAGS}")
  set(BATT_DEFAULT_LINKOPTS "${BATT_MSVC_LINKOPTS}")
else()
  message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER}.  Building with no default flags")
  set(BATT_DEFAULT_COPTS "")
  set(BATT_TEST_COPTS "")
endif()

if("${CMAKE_CXX_STANDARD}" EQUAL 98)
  message(FATAL_ERROR "Batteries requires at least C++11")
elseif(NOT "${CMAKE_CXX_STANDARD}")
  message(STATUS "No CMAKE_CXX_STANDARD set, assuming 11")
  set(BATT_CXX_STANDARD 11)
else()
  set(BATT_CXX_STANDARD "${CMAKE_CXX_STANDARD}")
endif()
