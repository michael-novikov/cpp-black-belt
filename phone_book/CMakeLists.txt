cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Release)

set(this_project phone_book)

set(utility ~/workspace/cpp-brown-belt/utility)
include_directories(${utility})

project(${this_project} CXX)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS contact.proto)

set(headers
  iterator_range.h
  phone_book.h
  )

set(sources
  phone_book.cpp
  main.cpp
  )

add_executable(${this_project} ${sources} ${headers} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(${this_project} ${Protobuf_LIBRARIES})

