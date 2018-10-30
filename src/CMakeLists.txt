cmake_minimum_required(VERSION 3.8)

project(reactor)

set(CMAKE_CXX_STANDARD 17)

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(reactor)

set(HEADER_FILES reactor/engine.h reactor/window.h reactor/scene.h)

set(SOURCE_FILES main.cpp reactor/engine.cpp reactor/window.cpp reactor/scene.cpp)

add_executable(reactor ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(reactor glfw ${Vulkan_LIBRARY} imgui)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT reactor)
