cmake_minimum_required(VERSION 3.28)


set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS TRUE)

project(Minesweeper)

set(SOURCES
    "src/minesweeper.c"
)

add_executable(${PROJECT_NAME} ${SOURCES})

set(INCLUDE_PATH "${CMAKE_SOURCE_DIR}/include")
set(LIB_PATH "${CMAKE_SOURCE_DIR}/lib")

message(STATUS "Using lib path: ${LIB_PATH}")
message(STATUS "Using include path: ${INCLUDE_PATH}")

# Link raylib static library
set(RAYLIB_BIN "${LIB_PATH}/libraylib.a")

target_link_libraries(${PROJECT_NAME}
    ${RAYLIB_BIN} m
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_PATH})
target_compile_options(${PROJECT_NAME} PRIVATE -g -std=gnu99 -Wall)
