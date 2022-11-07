cmake_minimum_required(VERSION 3.12)

project(JAAMConverter)

SETUP_APP(JAAMConverter "JAAM")
set_target_properties(JAAMConverter PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/jaam")

target_link_libraries(JAAMConverter PUBLIC JAAMLib lz4 nlohmann_json assimp)

target_include_directories(JAAMConverter PUBLIC ../lib/include)
target_include_directories(JAAMConverter SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/assimp/include)
target_include_directories(JAAMConverter SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/stb)
target_include_directories(JAAMConverter SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/lz4/lib)
target_include_directories(JAAMConverter SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/json/include)
target_include_directories(JAAMConverter SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/glm)

set_property(TARGET zlibstatic PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET assimp PROPERTY FOLDER "ThirdPartyLibraries")