cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

set(this_project mython)

set(utility ../utility)

include_directories(${utility})
include_directories(../lexer)

project(${this_project} CXX)

set(headers
  comparators.h
  object.h
  object_holder.h
  parse.h
  statement.h
  )

set(sources
  ../lexer/lexer.cpp
  ../lexer/lexer_test.cpp
  ../lexer/string_utils.cpp
  #comparators.cpp
  object.cpp
  object_holder.cpp
  object_holder_test.cpp
  object_test.cpp
  #parse.cpp
  #parse_test.cpp
  statement.cpp
  statement_test.cpp
  mython.cpp
  )

add_executable(${this_project} ${sources} ${headers})

