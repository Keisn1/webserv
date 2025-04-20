cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_COMPILER "c++")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

project(webserv)
option(BUILD_TEST "Build tests" ON)

set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -fsanitize=address")

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

if(BUILD_TEST)
  file(GLOB UNITTEST_FILES "tests/test_config_ast.cpp")
  set(UNITTEST_EXECUTABLE "run_unittests")

  add_executable(${UNITTEST_EXECUTABLE} ${SOURCES} ${UNITTEST_FILES})
  target_link_libraries(${UNITTEST_EXECUTABLE} gtest gtest_main pthread gmock)

  target_include_directories(${UNITTEST_EXECUTABLE} PRIVATE "./includes")

  include(GoogleTest)
  gtest_discover_tests(${UNITTEST_EXECUTABLE})
endif()

