cmake_minimum_required(VERSION 3.5.0)
project(fgport VERSION 0.1.0 LANGUAGES C CXX)

enable_testing()
include(CTest)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
add_compile_options(-DFDEBUG -g -ggdb -fPIC -O0)

include(FetchContent)
FetchContent_Declare(
    uv
    URL https://dist.libuv.org/dist/v1.48.0/libuv-v1.48.0.tar.gz
)

FetchContent_MakeAvailable(uv)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/build/${PROJECT_NAME}/_deps/uv-src/include)

# Add custom command to compile eBPF program
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/build/${PROJECT_NAME}/socket_ip_filter.o
    COMMAND clang -O2 -target bpf -g -c ${CMAKE_SOURCE_DIR}/src/ebpf/socket_ip_filter.c -o ${CMAKE_SOURCE_DIR}/build/${PROJECT_NAME}/socket_ip_filter.o
    DEPENDS ${CMAKE_SOURCE_DIR}/src/ebpf/socket_ip_filter.c
    COMMENT "compiling eBPF program"
)
add_custom_target(compile_ebpf ALL DEPENDS ${CMAKE_SOURCE_DIR}/build/${PROJECT_NAME}/socket_ip_filter.o)

add_executable(fgport main.cpp)
add_dependencies(fgport compile_ebpf)
target_link_libraries(fgport PUBLIC uv bpf)

add_subdirectory(src)
add_subdirectory(test)