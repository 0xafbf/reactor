
require "external/premake/export_compile_commands"
--- in theory, as premake is in the same repo as this, it shouldn't be a problem




target_location = "%{wks.location}/bin/%{cfg.shortname}"

workspace "reactor"

	configurations { "Debug", "Work", "Release" }
	platforms { "x86", "x64" }
	

	staticruntime "On"
	systemversion "latest"
	targetdir (target_location)
	

	filter { "platforms:x64" }
		architecture "x64"
	filter { "platforms:x86" }
		architecture "x86"


	filter "system:windows"
		defines { "WIN32" }
		defines { "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1"}
		systemversion "latest"

	filter "system:linux"
        defines {
            --"_GLFW_OSMESA", -- options: x11 osmesa wayland
            -- osmesa compiles installs without x11 or wayland headers
            "GLFW_INCLUDE_NONE", -- avoid including GL stuff
        }
        links { "X11", "Xrandr", "Xinerama", "Xi" , "Xcursor"} -- for x11


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







include "deps/glfw.lua"
include "deps/slang.lua"

group ""
project "reactor"
	systemversion "latest"
		
	kind "WindowedApp"
	language "C++"

	warnings "Off"


	pchheader "pch.h"
	pchsource "src/reactor/pch.cpp"

	forceincludes "pch.h"

	files { "premake5.lua" }
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
	links {"glfw"}
	includedirs { "deps/glfw/include"}
	
	includedirs { "deps/glfw/include"}
	files { "deps/SPIRV-Reflect/spirv_reflect.*" }
	includedirs { "deps/SPIRV-Reflect" }

	includedirs { "deps/slang" }

	links {"slang", "slang-glslang"}

		
	includedirs { "deps/slang" }
	files {"shaders/**.slang" }



	filter "system:windows"
		entrypoint "mainCRTStartup"

	filter {"system:windows", "platforms:x86"}
	       	links { "$(VULKAN_SDK)/lib32/vulkan-1" }
	filter {"system:windows", "platforms:x64"}
	       	links { "$(VULKAN_SDK)/lib/vulkan-1" }

	filter "system:linux"
	       	links { "$(VULKAN_SDK)/lib/vulkan", 
	       		"dl", "pthread" -- for glfw
	       	}
