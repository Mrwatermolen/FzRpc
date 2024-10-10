find_package(gRPC)

if (gRPC_FOUND)
    message(STATUS "gRPC found")
    set(_REFLECTION gRPC::grpc++_reflection)
    set(_GRPC_GRPCPP gRPC::grpc++)

    message(STATUS "Using gRPC ${gRPC_VERSION}")

    set(hello_world_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/hello_world.pb.cc")
    set(hello_world_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/hello_world.pb.h")
    set(hello_world_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/hello_world.grpc.pb.cc")
    set(hello_world_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/hello_world.grpc.pb.h")

    add_custom_command(
        OUTPUT "${hello_world_proto_srcs}" "${hello_world_proto_hdrs}" "${hello_world_grpc_srcs}" "${hello_world_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${CMAKE_CURRENT_SOURCE_DIR}"
        --plugin=protoc-gen-grpc="${_grpc_cpp_plugin}"
        "${CMAKE_CURRENT_SOURCE_DIR}/hello_world.proto"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/hello_world.proto")

    add_library(hello_world_proto
        "${hello_world_proto_srcs}"
        "${hello_world_grpc_srcs}")

    target_include_directories(hello_world_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(hello_world_proto PUBLIC ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

    add_executable(rpc_hello_world_server server.cpp)
    target_link_libraries(rpc_hello_world_server hello_world_proto)

    add_executable(rpc_hello_world_client client.cpp)
    target_link_libraries(rpc_hello_world_client hello_world_proto)
else()
    message(FATAL_ERROR "gRPC not found. Disabling rpc_hello_world_server and rpc_hello_world_client")
endif()


