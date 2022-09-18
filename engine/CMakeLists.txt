cmake_minimum_required(VERSION 3.12)

project(Engine)

include(../cmake/commonmacros.txt)

SETUP_LIB(Engine "Scorch")

target_include_directories(Engine PRIVATE vendor/src/vulkan/include)

target_precompile_headers(Engine PRIVATE src/pch.h)
target_link_libraries(Engine PRIVATE glfw)