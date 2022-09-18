cmake_minimum_required(VERSION 3.12)

project(Sandbox)

include(../cmake/commonmacros.txt)

SETUP_APP(Sandbox "Scorch")
set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/sandbox")

target_link_libraries(Sandbox Engine)
target_include_directories(Sandbox PRIVATE ../Engine/src)