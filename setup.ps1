
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

pop-location