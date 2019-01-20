

includeexternal "deps/slang"


workspace "reactor"

	configurations { "Debug", "Work", "Release" }
	platforms { "win32", "win64" }
	
	files { "premake5.lua" }

	filter "system:windows"
		defines { "WIN32" }
		defines { "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1"}
		systemversion "latest"
		entrypoint "mainCRTStartup"

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
	kind "WindowedApp"
	language "C++"

	warnings "Off"

	files { "src/**.h", "src/**.cpp" }
	includedirs { "src/reactor" }

	includedirs { "$(VULKAN_SDK)/include" }


	files { "deps/imgui/*.h", "deps/imgui/*.cpp" }
	files { "deps/imgui/examples/imgui_impl_glfw.*", "deps/imgui/examples/imgui_impl_vulkan.*"}
	includedirs { "deps/imgui" }

	files { "deps/tinyobjloader/tiny_obj_loader.*" }
	includedirs { "deps/tinyobjloader" }

	files { "deps/stb/stb_image.h" }
	files { "deps/stb/stb_image_write.h" }
	includedirs { "deps/stb" }

	-- files { "deps/glfw/"}
	-- links {"glfw"}
	includedirs { "deps/glfw/include"}
	links {"deps/glfw/glfw3"}
	
	includedirs { "deps/glfw/include"}
	files { "deps/SPIRV-Reflect/spirv_reflect.*" }
	includedirs { "deps/SPIRV-Reflect" }

	includedirs { "deps/slang" }

	links {"deps/slang/bin/windows-x64/release/slang",
		"deps/slang/bin/windows-x64/release/slang-glslang"}
	copylocal {"deps/slang/bin/windows-x64/release/slang",
		"deps/slang/bin/windows-x64/release/slang-glslang"}

	includedirs { "deps/slang" }
	files {"shaders/**.slang" }
	filter "files:skipshaders/basic.slang"
		buildmessage "Compiling %{file.relpath}"
		buildcommands {
			"%{wks.location}/shaders/bin/slangc %{file.path} -entry vert -o %{wks.location}shaders/%{file.basename}.vert.spv",
			"%{wks.location}/shaders/bin/slangc %{file.path} -entry frag -o %{wks.location}shaders/%{file.basename}.frag.spv"
		}
		buildoutputs {
			"%{wks.location}shaders/%{file.basename}.frag.spv",
			"%{wks.location}shaders/%{file.basename}.vert.spv"
		}

	filter "platforms:win32"
	       architecture "x86"
	       	links { "$(VULKAN_SDK)/lib32/vulkan-1" }


	filter "platforms:win64"
	       architecture "x86_64"
	       	links { "$(VULKAN_SDK)/lib/vulkan-1" }
