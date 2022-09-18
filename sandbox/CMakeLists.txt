cmake_minimum_required(VERSION 3.12)

project(Sandbox)

include(../cmake/commonmacros.txt)

SETUP_APP(Sandbox "Scorch")
set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/sandbox")

target_link_libraries(Sandbox Engine)
target_include_directories(Sandbox PRIVATE ../Engine/src)

get_target_property(ENGINE_BINARY_DIR Engine BINARY_DIR)
get_target_property(SANDBOX_BINARY_DIR Sandbox BINARY_DIR)
message(STATUS "Engine output dir: ${ENGINE_BINARY_DIR}/Engine.dll")
message(STATUS "Sandbox output dir: ${SANDBOX_BINARY_DIR}/Engine.dll")

add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ENGINE_BINARY_DIR}/Engine.dll
        ${SANDBOX_BINARY_DIR})