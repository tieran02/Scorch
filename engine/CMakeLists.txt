cmake_minimum_required(VERSION 3.12)

project(Engine)

include(../cmake/commonmacros.txt)

SETUP_LIB(Engine "Scorch")

target_include_directories(Engine PRIVATE vendor/src/vulkan/include)
target_include_directories(Engine PRIVATE include/scorch)
target_include_directories(Engine PRIVATE ${CMAKE_SOURCE_DIR}/JAAM/lib/include)

target_precompile_headers(Engine PRIVATE include/scorch/pch.h)

target_compile_definitions(Engine PRIVATE
  $<$<CONFIG:Debug>:SCORCH_DEBUG=1>
)

target_compile_options(Engine PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd4100>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_link_libraries(Engine PRIVATE glfw volk vk-bootstrap assimp JAAMLib)