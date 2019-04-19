#! /usr/bin/env pwsh

Param(
	[switch] $Debug = $false
	)

Push-Location $PSScriptRoot

if (-not $(Test-Path external/.git)) {
   Write-Host "Project isn't set up, loading setup.ps1"
   & ./setup.ps1
}
If ($IsWindows){
	if ($null -eq (Get-Command "msbuild" -ErrorAction SilentlyContinue)) {
		Write-Output "MSBuild is not available, importing Visual Studio environment...";
		& external/VsDevEnv.ps1   
	}
}
ElseIf ($IsLinux){
	if ($null -eq (Get-Command "g++" -ErrorAction SilentlyContinue)) {
		Write-Output "g++ not installed"
		Write-Output "for fedora, do 'dnf install gcc-c++'"
		exit 1
	}
}


if (-not $env:VULKAN_SDK){
	if ($(Test-Path vulkansdk)) {
		Write-Host "Found local sdk installation, setting up environment"
		$env:VULKAN_SDK = $(Get-Location).Path + "/vulkansdk/x86_64"
		$env:PATH = "${env:VULKAN_SDK}/bin:${env:PATH}"
		$env:LD_LIBRARY_PATH = "${env:VULKAN_SDK}/lib:${env:LD_LIBRARY_PATH}"
		$env:VK_LAYER_PATH = "${env:VULKAN_SDK}/etc/explicit_layer.d"
	}
	Else {
		Write-Host "Vulkan SDK was not found (env:VULKAN_SDK is not set). Aborting."
		Write-Host "Install Vulkan SDK from https://vulkan.lunarg.com/"
		exit 1
	}
}

If ($IsWindows) {
	$platform = "x64"
	if (-not ${env:ProgramFiles(x86)}){
		$platform = "win32"
	}

	& msbuild /m /p:Configuration=Work /p:Platform=$platform
}
ElseIf ($IsLinux) {
	If ($Debug){
		make config=debug_x64
	}
	Else {
		make config=work_x64
	}
}
Pop-Location
