workspace "reactor"
	configurations { "Debug", "Work", "Release" }
	
	files { "premake5.lua" }

	filter "system:windows"
		defines { "WIN32" }
		defines { "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1"}
		systemversion "latest"

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


	
project "reactor"
	kind "ConsoleApp"
	language "C++"

	warnings "Off"

	architecture "x86_64"

	files { "src/**.h", "src/**.cpp" }
	includedirs { "src/reactor" }

	links { "$(VULKAN_SDK)/lib/vulkan-1" }
	includedirs { "$(VULKAN_SDK)/include" }

	defines {"SLANG_STATIC"}  -- this is needed here so it doesn't look for __imp_ prefix
	links {"slang"}
	includedirs { "deps/slang" }

	files { "deps/imgui/*.h", "deps/imgui/*.cpp" }
	files { "deps/imgui/examples/imgui_impl_glfw.*", "deps/imgui/examples/imgui_impl_vulkan.*"}
	includedirs { "deps/imgui" }

	files { "deps/tinyobjloader/tiny_obj_loader.*" }
	includedirs { "deps/tinyobjloader" }

	files { "deps/stb/stb_image.h" }
	includedirs { "deps/stb" }

	-- files { "deps/glfw/"}
	-- links {"glfw"}
	includedirs { "deps/glfw/include"}
	links {"deps/glfw/glfw3"}
	
	files { "deps/SPIRV-Reflect/spirv_reflect.*" }
	includedirs { "deps/SPIRV-Reflect" }

	files { "deps/spdlog/include/spdlog/**" }
	includedirs { "deps/spdlog/include" }

	
	-- files {"shaders/**.slang" }
	filter "files:shaders/basic.slang"
		buildmessage "Compiling %{file.relpath}"
		buildcommands {
			"%{wks.location}/shaders/bin/slangc %{file.path} -entry vert -o %{wks.location}shaders/%{file.basename}.vert.spv",
			"%{wks.location}/shaders/bin/slangc %{file.path} -entry frag -o %{wks.location}shaders/%{file.basename}.frag.spv"
		}
		buildoutputs {
			"%{wks.location}shaders/%{file.basename}.frag.spv",
			"%{wks.location}shaders/%{file.basename}.vert.spv"
		}


group "dependencies"
			
	project "slang"
		kind "StaticLib"
		language "C++"

		location "deps/slang"
		architecture "x86_64"

		defines {"SLANG_STATIC"}

		files { "deps/slang/*.h"}
		files { "deps/slang/source/slang/*.cpp"}
		files { "deps/slang/source/slang/*.h"}
		files { "deps/slang/source/core/*.cpp"}
		files { "deps/slang/source/core/*.h"}
		files { "deps/slang/source/slang/slang.natvis" }
		includedirs {"deps/slang"}
