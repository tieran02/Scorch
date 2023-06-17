cmake_minimum_required(VERSION 3.24)

project(Engine)

include(../cmake/commonmacros.txt)

SETUP_LIB(Engine "Scorch")

target_include_directories(Engine PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/vulkan/include)
target_include_directories(Engine PRIVATE include/scorch)
target_include_directories(Engine PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/jaam/lib/include)

target_precompile_headers(Engine PRIVATE include/scorch/pch.h)

target_compile_definitions(Engine PUBLIC 
  $<$<CONFIG:Debug>:SCORCH_DEBUG=1>
  "RENDERER_D12"
)

set_property(TARGET Engine PROPERTY COMPILE_WARNING_AS_ERROR ON)

target_link_libraries(Engine PRIVATE d3d12.lib dxgi.lib dxguid.lib )
target_link_libraries(Engine PRIVATE glfw volk vk-bootstrap JAAMLib imgui)