
Push-Location $PSScriptRoot

if (-not ${Test-Path external/.git}) {
   Write-Host "Project isn't set up, loading setup.ps1"
   & ./setup.ps1
}

if ((Get-Command "msbuild" -ErrorAction SilentlyContinue) -eq $null) {
   echo "MSBuild is not available, importing Visual Studio environment...";
   & external/VsDevEnv.ps1   
}

# TODO check if VULKAN_SDK is in environment
if (-not $env:VULKAN_SDK){
   Write-Host "Vulkan SDK was not found (env:VULKAN_SDK is not set). Aborting."
   Write-Host "Install Vulkan SDK from https://vulkan.lunarg.com/"
   exit 1
}

$platform = "x64"
if (-not ${env:ProgramFiles(x86)}){
   $platform = "win32"
}

& msbuild /m /p:Configuration=Work /p:Platform=$platform

Pop-Location
