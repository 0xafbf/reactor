
# get submodules
echo "updating submodules..."

& git submodule update --init

echo "running premake..."

& ./external/premake/premake5 vs2017
