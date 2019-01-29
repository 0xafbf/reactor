
--- taken from https://github.com/TheCherno/glfw/blob/master/premake5.lua


os.chdir("glfw") -- just added this line

group "deps"
project "glfw"
	
	kind "StaticLib"
	language "C"
    systemversion "latest"
    
	files {
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
		"src/internal.h",
        "src/glfw_config.h",
        "src/context.c",
        "src/init.c",
        "src/input.c",
        "src/monitor.c",
        "src/vulkan.c",
        "src/window.c"
    }
    
	filter "system:windows"
        staticruntime "On"
        
        files
        {
            "src/win32_init.c",
            "src/win32_joystick.c",
            "src/win32_monitor.c",
            "src/win32_time.c",
            "src/win32_thread.c",
            "src/win32_window.c",
            "src/wgl_context.c",
            "src/egl_context.c",
            "src/osmesa_context.c"
        }

		defines 
		{ 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
		}
    filter { "system:windows", "configurations:Release" }
		   buildoptions "/MT"
