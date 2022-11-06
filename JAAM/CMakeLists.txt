cmake_minimum_required(VERSION 3.12)

#Just Another Asset Manager
project(JAAM)

include(${CMAKE_SOURCE_DIR}/cmake/commonmacros.txt)

add_subdirectory(lib)
add_subdirectory(converter)