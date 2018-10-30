
find_package(Vulkan)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
add_subdirectory(glfw)



set(imgui_HEADERS imgui/imconfig.h imgui/imgui.h imgui/imgui_internal.h 
				imgui/imstb_rectpack.h imgui/imstb_textedit.h imgui/imstb_truetype.h)
				
set(imgui_SOURCES imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp)

# these are useful conveniences
set(imgui_HEADERS ${imgui_HEADERS} imgui/examples/imgui_impl_vulkan.h imgui/examples/imgui_impl_glfw.h )
set(imgui_SOURCES ${imgui_SOURCES} imgui/examples/imgui_impl_vulkan.cpp imgui/examples/imgui_impl_glfw.cpp )

add_library(imgui ${imgui_SOURCES} ${imgui_HEADERS})
target_include_directories(imgui PUBLIC ${Vulkan_INCLUDE_DIR} ${glfw_INCLUDE_DIRS})
target_include_directories(imgui PUBLIC imgui)

target_link_libraries(imgui glfw)

set_target_properties(imgui PROPERTIES
                      #OUTPUT_NAME ${GLFW_LIB_NAME}
                      #VERSION ${GLFW_VERSION}
                      #SOVERSION ${GLFW_VERSION_MAJOR}
                      #POSITION_INDEPENDENT_CODE ON
                      FOLDER "imgui")

						