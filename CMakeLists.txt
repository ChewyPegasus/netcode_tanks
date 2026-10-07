cmake_minimum_required(VERSION 3.25)
project(netcode_demo VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(NETCODE_BUILD_TESTS "Build unit tests" ON)
option(NETCODE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(NETCODE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_library(netcode_options INTERFACE)
if(MSVC)
  target_compile_options(netcode_options INTERFACE /W4 /permissive-)
  if(NETCODE_WARNINGS_AS_ERRORS)
    target_compile_options(netcode_options INTERFACE /WX)
  endif()
else()
  target_compile_options(netcode_options INTERFACE -Wall -Wextra -Wpedantic -Wshadow)
  if(NETCODE_WARNINGS_AS_ERRORS)
    target_compile_options(netcode_options INTERFACE -Werror)
  endif()
  if(NETCODE_SANITIZERS)
    target_compile_options(netcode_options INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(netcode_options INTERFACE -fsanitize=address,undefined)
  endif()
endif()

add_library(netcode_core
  src/bitstream.cpp
  src/reliability.cpp)
target_include_directories(netcode_core PUBLIC include)
target_link_libraries(netcode_core PUBLIC netcode_options)

if(NETCODE_BUILD_TESTS)
  enable_testing()
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SYSTEM)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_executable(netcode_tests
    tests/test_bitstream.cpp
    tests/test_reliability.cpp
    tests/test_sequence.cpp)
  target_link_libraries(netcode_tests PRIVATE netcode_core GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(netcode_tests DISCOVERY_MODE PRE_TEST)
endif()
