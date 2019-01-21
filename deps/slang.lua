os.chdir("slang")

newoption {
	trigger     = "execute-binary",
	description = "(Optional) If true binaries used in build will be executed (disable on cross compilation)",
	value       = "bool",
	default     = "true",
	allowed     = { { "true", "True"}, { "false", "False" } }
}

executeBinary = (_OPTIONS["execute-binary"] == "true")

targetName = "%{cfg.system}-%{cfg.platform:lower()}"

workspace "slang"
	
	configurations { "Debug", "Release" }
	platforms { "x86", "x64"}
	
	if os.target() == "linux" then
		platforms {"aarch64" }
	end

	systemversion "latest"

	targetdir("bin/" .. targetName .. "/%{cfg.buildcfg:lower()}")

	flags { "StaticRuntime" }

	filter { "platforms:x64" }
		architecture "x64"
	filter { "platforms:x86" }
		architecture "x86"
	filter { "platforms:aarch64"}
		architecture "ARM"

	filter { "toolset:clang or gcc*" }
		buildoptions { "-Wno-unused-parameter", "-Wno-type-limits", "-Wno-sign-compare", "-Wno-unused-variable", "-Wno-reorder", "-Wno-switch", "-Wno-return-type", "-Wno-unused-local-typedefs", "-Wno-parentheses",  "-std=c++11", "-fvisibility=hidden" , "-fno-delete-null-pointer-checks", "-Wno-ignored-optimization-argument", "-Wno-unknown-warning-option"} 
		
	filter { "toolset:gcc*"}
		buildoptions { "-Wno-nonnull-compare", "-Wno-unused-but-set-variable", "-Wno-implicit-fallthrough"  }
		
	filter { "toolset:clang" }
		buildoptions { "-Wno-deprecated-register", "-Wno-tautological-compare", "-Wno-missing-braces", "-Wno-undefined-var-template", "-Wno-unused-function", "-Wno-undefined-bool-conversion"}
		
	filter { "configurations:debug" }
		optimize "Off"
		symbols "On"
		defines { "_DEBUG" }

	filter { "configurations:release" }
		optimize "On"
		defines { "NDEBUG" }
			
	filter { "system:linux" }
		linkoptions{  "-Wl,-rpath,'$$ORIGIN',--no-as-needed", "-ldl"}


function addSourceDir(sourceDir)
	
	files
	{
		sourceDir .. "/*.cpp",
		sourceDir .. "/*.slang",
		sourceDir .. "/*.h",
		sourceDir .. "/*.hpp",
		sourceDir .. "/*.natvis",
	}
end

group ""
project "core"
	addSourceDir("source/core")
	location "source"
	language "C++"
	uuid "F9BE7957-8399-899E-0C49-E714FDDD4B65"
	kind "StaticLib"
	-- warnings "Extra"
	-- flags { "FatalWarnings" }
	filter { "system:linux" }
		buildoptions{"-fPIC"}

group "tools"
project "slang-generate"
	addSourceDir("tools/slang-generate")
	location "tools"
	language "C++"
	kind "ConsoleApp"
	uuid "66174227-8541-41FC-A6DF-4764FC66F78E" -- uuid(os.uuid(name .. '|' .. sourceDir))
	links { "core" }

project "slang-test"
	addSourceDir("tools/slang-test")	
	location "tools"
	language "C++"
	kind "ConsoleApp"
	uuid "0C768A18-1D25-4000-9F37-DA5FE99E3B64"
	includedirs { "." }
	links { "core", "slang" }

group "test-tool"
project "slang-reflection-test-tool"
	addSourceDir("tools/slang-reflection-test")
	location "tools"
	language "C++"
	defines { "SLANG_SHARED_LIBRARY_TOOL" }
	kind "SharedLib"
	uuid "C5ACCA6E-C04D-4B36-8516-3752B3C13C2F"
	includedirs { "." }
	kind "SharedLib"
	links { "core", "slang" }
if os.target() == "windows" then
	group "test-tool"
	project "render-test-tool"
		addSourceDir("tools/render-test")
		location "tools"
		language "C++"
		defines { "SLANG_SHARED_LIBRARY_TOOL" }
		kind "SharedLib"
		uuid "61F7EB00-7281-4BF3-9470-7C2EA92620C3"
		includedirs { ".", "external", "source", "tools/gfx" }
		links { "core", "slang", "gfx" }
		postbuildcommands { '"$(SolutionDir)tools\\copy-hlsl-libs.bat" "$(WindowsSdkDir)Redist/D3D/%{cfg.platform:lower()}/" "%{cfg.targetdir}/"'}
end

group "tools"
project "gfx"
	addSourceDir("tools/gfx")
	location "tools"
	language "C++"
	kind "ConsoleApp"
	uuid "222F7498-B40C-4F3F-A704-DDEB91A4484A"
	kind "StaticLib"
	includedirs { ".", "external", "source", "external/imgui" }
	filter { "system:windows" }
		postbuildcommands { '"$(SolutionDir)tools\\copy-hlsl-libs.bat" "$(WindowsSdkDir)Redist/D3D/%{cfg.platform:lower()}/" "%{cfg.targetdir}/"'}
		filter { "system:not windows" }
			removefiles { "tools/gfx/circular-resource-heap-d3d12.cpp", "tools/gfx/d3d-util.cpp", "tools/gfx/descriptor-heap-d3d12.cpp", "tools/gfx/render-d3d11.cpp", "tools/gfx/render-d3d12.cpp", "tools/gfx/render-gl.cpp", "tools/gfx/resource-d3d12.cpp", "tools/gfx/render-vk.cpp", "tools/gfx/vk-swap-chain.cpp", "tools/gfx/window.cpp" }



group ""
project "slangc"
	addSourceDir("source/slangc")
	location "source"
	language "C++"
	uuid "D56CBCEB-1EB5-4CA8-AEC4-48EA35ED61C7"
	kind "ConsoleApp"
	links { "core", "slang" }

group ""
project "slang"
	addSourceDir("source/slang")
	location "source"
	language "C++"
	uuid "DB00DA62-0533-4AFD-B59F-A67D5B3A0808"
	kind "SharedLib"
	links { "core" }
	-- warnings "Extra"
	-- flags { "FatalWarnings" }
	defines { "SLANG_DYNAMIC_EXPORT" }
	files { "slang.h" }
	dependson { "slang-generate" }
	filter { "system:linux" }
		buildoptions{"-fPIC"}
	if executeBinary then
		filter "files:**.meta.slang"
			buildmessage "slang-generate %{file.relpath}"
			buildcommands { '"%{cfg.targetdir}/slang-generate" %{file.relpath}' }
			buildoutputs { "%{file.relpath}.h" }
			local executableSuffix = "";
			if(os.target() == "windows") then
				executableSuffix = ".exe";
			end

			buildinputs { "%{cfg.targetdir}/slang-generate" .. executableSuffix }
	end

group ""
project "slang-glslang"
	addSourceDir("source/slang-glslang")
	location "source"
	language "C++"
	uuid "C495878A-832C-485B-B347-0998A90CC936"
	kind "SharedLib"
	includedirs { "external/glslang" }
	defines
	{
		"ENABLE_OPT=0",
		"AMD_EXTENSIONS",
		"NV_EXTENSIONS",
	}
	files {
		"external/glslang/glslang/GenericCodeGen/*",
		"external/glslang/glslang/MachineIndependent/**",
		"external/glslang/glslang/OSDependent/*",
		"external/glslang/OGLCompilersDLL/*",
		"external/glslang/SPIRV/*",
		"external/glslang/StandAlone/*"
	}
	removefiles { 
		"external/glslang/StandAlone/StandAlone.cpp" 
	}
	filter { "system:windows" }
		files { "external/glslang/glslang/OSDependent/Windows/*" }
		removefiles { "external/glslang/glslang/OSDependent/Windows/main.cpp" }
	filter { "system:linux" }
		links { "dl", "pthread" }
		files {"external/glslang/glslang/OSDependent/Unix"}
		buildoptions{"-fPIC", "-pthread"}
