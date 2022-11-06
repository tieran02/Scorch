cmake_minimum_required(VERSION 3.12)

project(AssetLib)

include(${CMAKE_SOURCE_DIR}/cmake/commonmacros.txt)

SETUP_LIB(AssetLib "AssetLib")

target_include_directories(AssetLib PRIVATE include)

target_compile_options(AssetLib PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd4100>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

# assimp settings
#set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
#set(ASSIMP_INSTALL OFF CACHE BOOL "")
#set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")
#set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "")
#set(BUILD_SHARED_LIBS ON CACHE BOOL "")
#add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/src/assimp)

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/src/json json)

add_library(lz4 STATIC)
target_sources(lz4 PRIVATE 
    "${CMAKE_SOURCE_DIR}/vendor/src/lz4/lib/lz4.h"
    "${CMAKE_SOURCE_DIR}/vendor/src/lz4/lib/lz4.c"
)

target_include_directories(AssetLib SYSTEM PRIVATE vendor/src/assimp/include)
target_include_directories(AssetLib SYSTEM PRIVATE vendor/src/stb)
target_include_directories(AssetLib SYSTEM PRIVATE vendor/src/lz4/lib)
target_include_directories(AssetLib SYSTEM PRIVATE vendor/src/json/include)

target_link_libraries(AssetLib PRIVATE lz4 json assimp)

set_property(TARGET zlibstatic PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET assimp PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET lz4 PROPERTY FOLDER "ThirdPartyLibraries")
#set_property(TARGET json PROPERTY FOLDER "ThirdPartyLibraries")