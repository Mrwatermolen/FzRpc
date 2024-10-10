aux_source_directory(. FZ_RPC_SOURCES)

add_library(fz_rpc ${FZ_RPC_SOURCES})

target_include_directories(fz_rpc PUBLIC $<BUILD_INTERFACE:${FZ_RPC_PUBLIC_INCLUDE_DIRS}> $<INSTALL_INTERFACE:include>)
target_include_directories(fz_rpc PUBLIC $<BUILD_INTERFACE:${FZ_RPC_EXTERNAL_INCLUDE_DIRS}> $<INSTALL_INTERFACE:include>)
target_link_libraries(fz_rpc PUBLIC $<BUILD_INTERFACE:${FZ_RPC_EXTERNAL_LIBRARIES}> $<INSTALL_INTERFACE:lib>)
