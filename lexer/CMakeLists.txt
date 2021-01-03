cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

set(this_project lexer)

set(utility ../utility)
include_directories(${utility})

add_definitions(-DLEXER_EXECUTABLE)

project(${this_project} CXX)

set(headers
  string_utils.h
  lexer.h
  )

set(sources
  string_utils.cpp
  lexer.cpp
  lexer_test.cpp
  )

add_executable(${this_project} ${sources} ${headers})

