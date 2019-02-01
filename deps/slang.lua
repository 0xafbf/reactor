os.chdir("slang")

newoption {
	trigger     = "execute-binary",
	description = "(Optional) If true binaries used in build will be executed (disable on cross compilation)",
	value       = "bool",
	default     = "true",
	allowed     = { { "true", "True"}, { "false", "False" } }
}

executeBinary = (_OPTIONS["execute-binary"] == "true")


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

group "deps/slang"
project "core"
	addSourceDir("source/core")
	location "source"
	language "C++"
	uuid "F9BE7957-8399-899E-0C49-E714FDDD4B65"
	kind "StaticLib"
	filter { "system:linux" }
		buildoptions{"-fPIC"}

group "deps/slang"
project "slang-generate"
	addSourceDir("tools/slang-generate")
	location "tools"
	language "C++"
	kind "ConsoleApp"
	uuid "66174227-8541-41FC-A6DF-4764FC66F78E" -- uuid(os.uuid(name .. '|' .. sourceDir))
	links { "core" }
	targetdir(target_location)

group "deps/slang"
project "slang"
	addSourceDir("source/slang")
	location "source"
	language "C++"
	uuid "DB00DA62-0533-4AFD-B59F-A67D5B3A0808"
	kind "SharedLib"
	links { "core" }
	targetdir(targetName)

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

			buildinputs { "%{cfg.targetdir}/slang-generate"}
	end

group "deps/slang"
project "slang-glslang"
	addSourceDir("source/slang-glslang")
	location "source"
	language "C++"
	uuid "C495878A-832C-485B-B347-0998A90CC936"
	kind "SharedLib"
	includedirs { "external/glslang" }

	pchheader "pch.h"
	pchsource "external/glslang/glslang/MachineIndependent/pch.cpp"

	forceincludes "pch.h"
	includedirs { "external/glslang/glslang/MachineIndependent" }

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


