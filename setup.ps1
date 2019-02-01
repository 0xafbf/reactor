
Push-Location $PSScriptRoot

# get submodules
echo "updating submodules..."
echo "TODO: this takes a long time, maybe have some cache in place?"
& git submodule update --init

push-location "deps/slang"
& git submodule update --init external/glslang
pop-location



echo "running premake..."
& ./external/premake/premake5 vs2017 --verbose

pop-location