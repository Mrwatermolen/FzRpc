cmake_minimum_required(VERSION 3.20)

project(FzRpc VERSION 0.0.0 LANGUAGES CXX)

set(FzRpc_MAIN_PROJECT OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(FzRpc_MAIN_PROJECT ON)
endif()

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

find_package(Protobuf REQUIRED CONFIG)
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
find_program(_PROTOBUF_PROTOC protoc)
find_program(_grpc_cpp_plugin grpc_cpp_plugin)
message(STATUS "Using protoc ${_PROTOBUF_PROTOC}")
message(STATUS "Using grpc_cpp_plugin ${_grpc_cpp_plugin}")
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(FZ_NET_MODULE_PATH ${PROJECT_SOURCE_DIR}/../FzNet/build/Debug-arm)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${FZ_NET_MODULE_PATH})
message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
find_package(fz_net REQUIRED)

set(FZ_RPC_PUBLIC_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
set(FZ_RPC_EXTERNAL_INCLUDE_DIRS)
set(FZ_RPC_EXTERNAL_LIBRARIES ${_PROTOBUF_LIBPROTOBUF} fz::fz_net)

add_subdirectory(src)
add_subdirectory(test)
