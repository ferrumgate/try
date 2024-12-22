file(GLOB_RECURSE SOURCES "*.cpp")

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE LIB_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_library(
    ferrum
    ${LIB_SOURCES}
)

add_executable(
    hello_test
    ${SOURCES}
)
target_link_libraries(
    hello_test
    GTest::gtest_main
    ferrum
    uv
)

include(GoogleTest)
gtest_discover_tests(hello_test)
