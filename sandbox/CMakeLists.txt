cmake_minimum_required(VERSION 3.12)

project(Sandbox)

include(../cmake/commonmacros.txt)

SETUP_APP(Sandbox "Scorch")
set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/sandbox")

target_link_libraries(Sandbox Engine)
target_include_directories(Sandbox PRIVATE ../Engine/include)

get_target_property(ENGINE_BINARY_DIR Engine BINARY_DIR)
get_target_property(SANDBOX_BINARY_DIR Sandbox BINARY_DIR)
message(STATUS "Engine output dir: ${ENGINE_BINARY_DIR}/Engine.dll")
message(STATUS "Sandbox output dir: ${SANDBOX_BINARY_DIR}/Engine.dll")

#add_custom_command(TARGET Sandbox POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${ENGINE_BINARY_DIR}/Engine.dll
#        ${SANDBOX_BINARY_DIR})

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

## find all the shader files under the shaders folder
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp"
    )

## iterate each shader
foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${SANDBOX_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  ##execute glslang command to compile that specific shader
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )