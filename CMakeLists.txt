cmake_minimum_required(VERSION 3.12)

project(Scorch CXX C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
	set(PYTHON_EXECUTABLE "python")
else()
	set(PYTHON_EXECUTABLE "python3")
endif()

if((NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/.bootstrap.json") OR ("${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/deps/.bootstrap.json"))
	execute_process(
		COMMAND ${PYTHON_EXECUTABLE} "bootstrap.py"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE bootstrap_result
	)
	if (NOT (${bootstrap_result} EQUAL 0))
		message(FATAL_ERROR "Cannot run bootstrap.py")
	endif()
endif()

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

add_subdirectory(vendor/src/glfw)
add_subdirectory(vendor/src/vkbootstrap)
add_subdirectory(vendor/src/volk)

add_subdirectory(Engine)
add_subdirectory(Sandbox)

target_include_directories(Engine PRIVATE vendor/src/vulkan/include)
target_include_directories(Engine PRIVATE vendor/src/volk)
target_include_directories(Engine PRIVATE vendor/src/glfw/include)
target_include_directories(Engine PRIVATE vendor/src/spdlog/include)
target_include_directories(Engine PRIVATE vendor/src/vkbootstrap/src)

set(VK_BOOTSTRAP_VULKAN_HEADER_DIR vendor/src/vulkan/include)

set_property(TARGET glfw          PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET vk-bootstrap  PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET volk          PROPERTY FOLDER "ThirdPartyLibraries")