cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "clang")

# set the project name
project(main)

# add the MathFunctions library
add_subdirectory(libtga-master)

# add the executable
add_executable(${PROJECT_NAME} main.c functions.c)

target_link_libraries(${PROJECT_NAME} PUBLIC libtga PRIVATE m)

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
target_include_directories(${PROJECT_NAME} PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/libtga-master"
                          )

target_compile_options(${PROJECT_NAME} PUBLIC -Wall -pedantic -g)