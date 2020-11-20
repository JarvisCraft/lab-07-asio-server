cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_COVERAGE "Build coverage" OFF)

set(
        HUNTER_CACHE_SERVERS
        "https://github.com/bmstu-iu8-cpp-sem-3/hunter-cache"
        CACHE STRING "Default cache server"
)

include("tools/gate/cmake/HunterGate.cmake")

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.284.tar.gz"
        SHA1 "b15e14b31d96ee2e2be32187d21e2390d0b974c8"
)

project(simple-server-lib)
project(simple-server-app)
string(APPEND CMAKE_CXX_FLAGS " -pedantic -Werror -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS " -Wno-unused-command-line-argument")
string(APPEND CMAKE_CXX_FLAGS " -Wshadow -Wnon-virtual-dtor")

hunter_add_package(asio)
find_package(asio CONFIG REQUIRED)
hunter_add_package(Boost COMPONENTS log filesystem thread)
find_package(Boost CONFIG REQUIRED log filesystem thread)
hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

file(GLOB_RECURSE SIMPLE_SERVER_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/sources/simple_server_lib/*.cpp")
add_library(simple-server-lib STATIC ${SIMPLE_SERVER_LIB_SOURCES})
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
add_executable(tests ${TEST_SOURCES})
file(GLOB_RECURSE SIMPLE_SERVER_APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/sources/simple_server_app/*.cpp")
add_executable(simple-server-app ${SIMPLE_SERVER_APP_SOURCES})

target_include_directories(simple-server-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(simple-server-lib PUBLIC asio::asio_headeronly Boost::log)
target_link_libraries(tests PUBLIC GTest::gtest_main simple-server-lib)
target_link_libraries(simple-server-app PUBLIC simple-server-lib asio::asio_static Boost::log Boost::filesystem Boost::thread)

enable_testing()
add_test(NAME unit_tests COMMAND tests)

if (BUILD_COVERAGE)
    set(ENABLE_COVERAGE ON CACHE BOOL "Enable coverage build." FORCE)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/CMake-codecov/cmake")
    find_package(codecov)
    add_coverage(simple-server-lib)
    add_coverage(simple-server-app)
    add_coverage(tests)
    list(APPEND LCOV_REMOVE_PATTERNS "'${PROJECT_SOURCE_DIR}/tests/*'")
    coverage_evaluate()
endif ()
