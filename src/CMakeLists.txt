cmake_minimum_required(VERSION 3.8)

project(reactor)

set(CMAKE_CXX_STANDARD 17)

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(reactor)

set(SOURCE_FILES main.cpp)

add_executable(reactor ${SOURCE_FILES})

target_link_libraries(reactor glfw ${Vulkan_LIBRARY})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT reactor)
