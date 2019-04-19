#! /usr/bin/env pwsh

Push-Location $PSScriptRoot

# get submodules

if (-not $(Test-Path external/.git)) {
	Write-Host "updating submodules..."
	& git submodule update --init

	Push-Location "deps/slang"
	& git submodule update --init external/glslang
	Pop-Location
}

echo "running premake..."
If ($IsWindows) {
	& ./external/premake/premake5 vs2017 --verbose
}
ElseIf ($IsLinux) {
	& ./external/premake/premake5 gmake2 --verbose
}
# for linux, ./external/premake/premake5 gmake2 works well

echo "compilation units set up"


# required dependencies for linux:
# dnf install g++
# dnf install libX11-devel
# dnf install libXcursor-devel
# dnf install libXrandr-devel
# dnf install libXinerama-devel
# dnf install libXi-devel



# follow steps from here:
if ($IsLinux) {
	if (-not $env:VULKAN_SDK) {
		echo "VULKAN_SDK variable not set, setting up local installation"
		if (-not $(Test-Path vulkansdk)) {
			Invoke-WebRequest -Uri "https://sdk.lunarg.com/sdk/download/1.1.106.0/linux/vulkansdk-linux-x86_64-1.1.106.0.tar.gz" -OutFile "vulkansdk.tar.gz"
			tar -xzf "vulkansdk.tar.gz "
			mv "1.1.106.0" "vulkansdk"
		}
	}
}


pop-location