cmake_minimum_required(VERSION 4.0)
project(Project1)
include(FetchContent)

set(CMAKE_CXX_STANDARD 26)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(cs213-project1 cs213_tests.cpp)
target_link_libraries(cs213-project1 PRIVATE Catch2::Catch2WithMain)

FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG main
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

target_link_libraries(cs213-project1 PRIVATE glaze::glaze)

add_subdirectory(deps/libpqxx build-pqxx)

target_link_libraries(cs213-project1 PRIVATE pqxx)

FIND_PACKAGE(PostgreSQL REQUIRED)
INCLUDE_DIRECTORIES(${PostgreSQL_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(cs213-project1 PRIVATE ${PostgreSQL_LIBRARIES})

target_link_libraries(cs213-project1 PRIVATE stdc++ stdc++exp)