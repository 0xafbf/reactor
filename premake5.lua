workspace "reactor"
	configurations { "Debug", "Work", "Release" }

project "reactor"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin"

	files { "src/**.h", "src/**.cpp" }
	includedirs { "src/reactor" }

	links { "$(VULKAN_SDK)/lib/vulkan-1.lib" }
	includedirs { "$(VULKAN_SDK)/include" }

	files { "deps/imgui/*.h", "deps/imgui/*.cpp" }
	files { "deps/imgui/examples/imgui_impl_glfw.*", "deps/imgui/examples/imgui_impl_vulkan.*"}
	includedirs { "deps/imgui" }

	files { "deps/tinyobjloader/tiny_obj_loader.*" }
	includedirs { "deps/tinyobjloader" }

	-- files { "deps/glfw/"}
	links { "deps/glfw/lib/glfw3" }
	includedirs { "deps/glfw/include" }
	
	files { "deps/SPIRV-Reflect/spirv_reflect.*" }
	includedirs { "deps/SPIRV-Reflect" }
	
	architecture "x86_64"
		
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Work"
		defines { "DEBUG" }
		symbols "On"
		optimize "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

	filter "system:windows"
		defines { "WIN32" }
		defines { "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1"}

	files {"shaders/**.slang" }
	filter "files:shaders/**.slang"
		buildmessage "Compiling %{file.relpath}"
		buildcommands {
			"%{wks.location}/shaders/bin/slangc.exe %{file.path} -entry vert -o %{wks.location}shaders/%{file.basename}.vert.spv",
			"%{wks.location}/shaders/bin/slangc.exe %{file.path} -entry frag -o %{wks.location}shaders/%{file.basename}.frag.spv"
		}
		buildoutputs {
			"%{wks.location}shaders/%{file.basename}.frag.spv",
			"%{wks.location}shaders/%{file.basename}.vert.spv"
		}


