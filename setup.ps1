
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
& ./external/premake/premake5 vs2017 --verbose
# for linux, ./external/premake/premake5 gmake2 works well


# required dependencies for linux:
# dnf install g++

# follow steps from here:

# also, you need to have vulkan sdk
# if $env:VULKAN_SDK
# source $env:VULKAN_SDK/
# else {
# download "https://sdk.lunarg.com/sdk/download/1.1.106.0/linux/vulkansdk-linux-x86_64-1.1.106.0.tar.gz"
# tar -xzf vulkansdk-linux-x86_64-1.1.106.0.tar.gz
#mv 1.1.106.0 vulkansdk
# $env:VULKAN_SDK = $(Get-Location).Path + "/vulkansdk/x86_64"
# $env:PATH = "${env:VULKAN_SDK}/bin:${env:PATH}"
##$env:LD_LIBRARY_PATH = "${env:VULKAN_SDK}/lib:${env:LD_LIBRARY_PATH}"
##$env:VK_LAYER_PATH = "${env:VULKAN_SDK}/etc/explicit_layer.d"
##
##
#}



pop-location