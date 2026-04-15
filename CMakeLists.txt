cmake_minimum_required(VERSION 3.16)

project(HexFlayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(COMP_DIR ${SRC_DIR}/components)

file(GLOB SRC_FILES
    ${SRC_DIR}/*.cpp
    ${COMP_DIR}/*.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_DIR})