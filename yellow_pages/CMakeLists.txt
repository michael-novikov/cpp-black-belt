cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Release)

set(this_project yellow_pages)
project(${this_project} CXX)

find_package(Protobuf REQUIRED)
find_path(TEST_RUNNER_PATH test_runner.h)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${TEST_RUNNER_PATH})

protobuf_generate_cpp(
    PROTO_SRCS
    PROTO_HDRS
    protos/address.proto
    protos/company.proto
    protos/name.proto
    protos/phone.proto
    protos/provider.proto
    protos/signal.proto
    protos/url.proto
    protos/working_time.proto
)

add_executable(${this_project} ${PROTO_SRCS} ${PROTO_HDRS} test.cpp merge.cpp)
target_link_libraries(${this_project} ${Protobuf_LIBRARIES})
