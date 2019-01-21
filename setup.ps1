
# get submodules
echo "updating submodules..."
# this takes a long time, maybe have some cache in place?
#& git submodule update --init

echo "running premake..."

& ./external/premake/premake5 vs2017 --verbose
