
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
        
        -- some missing header files here?

        files {
            "src/win32_init.c",
            "src/win32_joystick.c",
            "src/win32_monitor.c",
            "src/win32_time.c",
            "src/win32_thread.c",
            "src/win32_window.c",
            "src/wgl_context.c",
            "src/egl_context.c",
            "src/osmesa_context.c" -- are these three really needed?
        }

		defines { 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
		}
    filter { "system:windows", "configurations:Release" }
		   buildoptions "/MT"

    filter "system:linux"
        files {  -- these are for the x11 backend
            "src/x11_platform.h",
            "src/xkb_unicode.h",
            "src/posix_time.h",
                                 
            "src/posix_thread.h",
            "src/glx_context.h",
            "src/egl_context.h",
            "src/osmesa_context.h",
            "src/linux_joystick.h",

            
            "src/x11_init.c",
            "src/x11_monitor.c",
            "src/x11_window.c",
                     
            "src/xkb_unicode.c",
            "src/posix_time.c",
            "src/posix_thread.c",
            "src/glx_context.c",
                     
            "src/egl_context.c",
            "src/osmesa_context.c",
            "src/linux_joystick.c",

        } 
        defines {
            --"_GLFW_OSMESA", -- for headless usage without windows
            "_GLFW_X11",
            --"_GLFW_WAYLAND",
        }
        undefines {
            "GLFW_INCLUDE_NONE",
        }
        links { "dl", "pthread" }
        links { "X11", "Xrandr", "Xinerama", "Xi" , "Xkb", "Xcursor"} -- for x11
