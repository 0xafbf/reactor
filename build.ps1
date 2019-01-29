
if ((Get-Command "msbuild" -ErrorAction SilentlyContinue) -eq $null) {
   echo "msbuild is not available, import visual studio environment";
   echo "you can get the environment by running:"
   echo "C:\Program Files\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat"
   exit 1;
}

# TODO check if VULKAN_SDK is in environment

# 
# & msbuild /m /p:Configuration=Work

& msbuild /m /p:Configuration=Work /p:Platform=win64

