cmake_minimum_required(VERSION 3.12)

project(Engine)

include(../cmake/commonmacros.txt)

SETUP_LIB(Engine "Scorch")

target_include_directories(Engine PRIVATE vendor/src/vulkan/include)
target_include_directories(Engine PRIVATE include)

target_precompile_headers(Engine PRIVATE include/pch.h)

target_compile_definitions(Engine PRIVATE
  $<$<CONFIG:Debug>:SCORCH_DEBUG=1>
)

target_link_libraries(Engine PRIVATE glfw volk vk-bootstrap)