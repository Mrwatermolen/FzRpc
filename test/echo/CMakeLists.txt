set(echo_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/echo.pb.cc")
set(echo_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/echo.pb.h")

add_custom_command(
    OUTPUT "${echo_proto_srcs}" "${echo_proto_hdrs}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
    -I "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/echo.proto"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/echo.proto")

add_library(echo_proto
    "${echo_proto_srcs}")

target_include_directories(echo_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(echo_proto PUBLIC ${_PROTOBUF_LIBPROTOBUF})

add_executable(echo_server server.cpp)
target_link_libraries(echo_server echo_proto fz_rpc)

add_executable(echo_client client.cpp)
target_link_libraries(echo_client echo_proto fz_rpc)
