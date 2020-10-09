cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0")


set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

set(this_project sanitize_transport_guide)

set(utility ~/workspace/cpp-brown-belt/utility)
include_directories(${utility})

project(${this_project} CXX)

set(headers
  descriptions.h
  graph.h
  json.h
  requests.h
  router.h
  sphere.h
  transport_catalog.h
  transport_router.h
  utils.h
  )

set(sources
  descriptions.cpp
  json.cpp
  requests.cpp
  sphere.cpp
  transport_catalog.cpp
  transport_router.cpp
  utils.cpp
  main.cpp
  )

add_executable(${this_project} ${sources} ${headers})

