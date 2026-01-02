workspace "RenderCore"
	architecture "x64"
	startproject "TestApp"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

local assimp_bin_dir = "Libraries/bin"
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
local vulkan_sdk = os.getenv("VULKAN_SDK")
if vulkan_sdk == nil then
    print("[Engine] WARNING: VULKAN_SDK environment variable not set!")
    print("          Vulkan will NOT be included in the build.")
else
    print("[Engine] Using Vulkan SDK at: " .. vulkan_sdk)
end

project "EngineUtils"
	location "Engine/Engine/EngineUtils"
	kind "staticlib"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"Engine/Engine/%{prj.name}/**.h",
		"Engine/Engine/%{prj.name}/**.hpp",
		"Engine/Engine/%{prj.name}/**.inl",
		"Engine/Engine/%{prj.name}/**.cpp",
		"Engine/Engine/%{prj.name}/**.c",
	}

	includedirs {
		"Libraries/include",
	}

	libdirs {}

	links {}

	defines {
		"VLE_UTILS_BUILD_DLL",
	}

	if vulkan_sdk ~= nil then
        includedirs {
            vulkan_sdk .. "/Include"
        }

        libdirs {
            vulkan_sdk .. "/Lib"
        }

        links {
            "vulkan-1.lib"
        }

        defines {
            "USE_VULKAN"
        }
    end

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "VLE_DEBUG"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release"
		defines "VLE_RELEASE"
		optimize "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "VLE_DIST"
		optimize "on"
		runtime "Release"

project "EngineBackend"
	location "Engine/Engine/EngineBackend"
	kind "staticlib"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"Engine/Engine/%{prj.name}/**.h",
		"Engine/Engine/%{prj.name}/**.hpp",
		"Engine/Engine/%{prj.name}/**.inl",
		"Engine/Engine/%{prj.name}/**.cpp",
		"Engine/Engine/%{prj.name}/**.c",
	}


	includedirs {
		"Libraries/include",
		"Engine/Engine/EngineUtils"
	}

	libdirs {
		"Libraries/lib",
	}

	links {
		"glfw3_mt.lib",
		"assimp-vc143-mtd.lib",
		"EngineUtils"
	}

	dependson { 
		"EngineUtils" 
	}

	defines {
		"VLE_BUILD_DLL",
	}

	if vulkan_sdk ~= nil then
        includedirs {
            vulkan_sdk .. "/Include"
        }

        libdirs {
            vulkan_sdk .. "/Lib"
        }

        links {
            "vulkan-1.lib"
        }

        defines {
            "USE_VULKAN"
        }
    end

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
	    links { "assimp-vc143-mtd.lib" }
		defines "VLE_DEBUG"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release"
	    links { "assimp-vc143-mt.lib" }
		defines "VLE_RELEASE"
		optimize "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "VLE_DIST"
		optimize "on"
		runtime "Release"

project "EngineSystems"
	location "Engine/Engine/Systems"
	kind "staticlib"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"Engine/Engine/Systems/**.h",
		"Engine/Engine/Systems/**.hpp",
		"Engine/Engine/Systems/**.inl",
		"Engine/Engine/Systems/**.cpp",
		"Engine/Engine/Systems/**.c",
	}

	includedirs {
		"Libraries/include",
		"Engine/Engine/EngineBackend",
		"Engine/Engine/EngineUtils"
	}

	libdirs {
		"Libraries/lib",
	}

	links {
		"EngineBackend",
		"EngineUtils"
	}

	dependson { 
		"EngineBackend", 
		"EngineUtils" 
	}

	if vulkan_sdk ~= nil then
        includedirs {
            vulkan_sdk .. "/Include"
        }

        libdirs {
            vulkan_sdk .. "/Lib"
        }

        links {
            "vulkan-1.lib"
        }

        defines {
            "USE_VULKAN"
        }
    end

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "APP_VLE_DEBUG"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release"
		defines "APP_VLE_RELEASE"
		optimize "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "APP_VLE_DIST"
		optimize "on"
		runtime "Release"

project "RayTracing"
	location "RayTracing"
	kind "staticlib"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.hpp",
		"%{prj.name}/**.inl",
		"%{prj.name}/**.cpp",
	}

	includedirs {
		"Libraries/include",
		"Engine/Engine/EngineBackend",
		"Engine/Engine/EngineUtils",
		"Engine/Engine/Systems"
	}

	libdirs {
		"Libraries/lib",
	}

	links {
		"EngineBackend",
		"EngineUtils",
		"EngineSystems",
	}

	dependson { 
		"EngineBackend", 
		"EngineUtils",
		"EngineSystems"
	}

	if vulkan_sdk ~= nil then
        includedirs {
            vulkan_sdk .. "/Include"
        }

        libdirs {
            vulkan_sdk .. "/Lib"
        }

        links {
            "vulkan-1.lib"
        }

        defines {
            "USE_VULKAN"
        }
    end

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "APP_DEBUG"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release"
		defines "APP_RELEASE"
		optimize "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "APP_DIST"
		optimize "on"
		runtime "Release"

project "GSplats"
	location "GSplats"
	kind "staticlib"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.hpp",
		"%{prj.name}/**.inl",
		"%{prj.name}/**.cpp",
	}

	includedirs {
		"Libraries/include",
		"Engine/Engine/EngineBackend",
		"Engine/Engine/EngineUtils",
		"Engine/Engine/Systems"
	}

	libdirs {
		"Libraries/lib",
	}

	links {
		"EngineBackend",
		"EngineUtils",
		"EngineSystems"
	}

	dependson { 
		"EngineBackend", 
		"EngineUtils",
		"EngineSystems"
	}

	if vulkan_sdk ~= nil then
        includedirs {
            vulkan_sdk .. "/Include"
        }

        libdirs {
            vulkan_sdk .. "/Lib"
        }

        links {
            "vulkan-1.lib"
        }

        defines {
            "USE_VULKAN"
        }
    end

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "APP_DEBUG"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release"
		defines "APP_RELEASE"
		optimize "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "APP_DIST"
		optimize "on"
		runtime "Release"

project "minmap-core"
	location "minmap-core"
	kind "staticlib"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.hpp",
		"%{prj.name}/**.inl",
		"%{prj.name}/**.cpp",
		"%{prj.name}/**.cc",
	}

	includedirs {
		"Libraries/include",
	}

	libdirs {
		"Libraries/lib",
	}

	links {
        "ceres.lib",
        "glog.lib"
	}

	dependson {}

	if vulkan_sdk ~= nil then
        includedirs {
            vulkan_sdk .. "/Include"
        }

        libdirs {
            vulkan_sdk .. "/Lib"
        }

        links {
            "vulkan-1.lib"
        }

        defines {
            "USE_VULKAN"
        }
    end

	filter "system:windows"
		systemversion "latest"
		defines { "_CRT_SECURE_NO_WARNINGS", "GLOG_NO_ABBREVIATED_SEVERITIES", "GLOG_USE_GLOG_EXPORT" }
		buildoptions { "/permissive-" }  -- <-- important

	filter "configurations:Debug"
		defines "APP_DEBUG"
		symbols "on"
		runtime "Debug"

project "minmap"
	location "minmap"
	kind "staticlib"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.hpp",
		"%{prj.name}/**.inl",
		"%{prj.name}/**.cpp",
		"%{prj.name}/**.cc",
	}

	includedirs {
		"Libraries/include",
		"minmap-core",
	}

	libdirs {
		"Libraries/lib",
		"minmap-core",
	}

	links {
        "ceres.lib",
        "glog.lib",
		"minmap-core",
	}

	dependson {}

	if vulkan_sdk ~= nil then
        includedirs {
            vulkan_sdk .. "/Include"
        }

        libdirs {
            vulkan_sdk .. "/Lib"
        }

        links {
            "vulkan-1.lib"
        }

        defines {
            "USE_VULKAN"
        }
    end

	filter "system:windows"
		systemversion "latest"
		defines { "_CRT_SECURE_NO_WARNINGS", "GLOG_NO_ABBREVIATED_SEVERITIES", "GLOG_USE_GLOG_EXPORT" }
		buildoptions { "/permissive-" }  -- <-- important

	filter "configurations:Debug"
		defines "APP_DEBUG"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release"
		defines "APP_RELEASE"
		optimize "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "APP_DIST"
		optimize "on"
		runtime "Release"

project "TestApp"
	location "TestApp"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.hpp",
		"%{prj.name}/**.inl",
		"%{prj.name}/**.cpp",
	}

	includedirs {
		"Libraries/include",
		"Engine/Engine/EngineBackend",
		"Engine/Engine/EngineUtils",
		"Engine/Engine/Systems",
		"RayTracing",
		"GSplats",
		"minmap"
	}

	libdirs {
		"Libraries/lib",
	}

	links {
		"EngineBackend",
		"EngineUtils",
		"EngineSystems",
		"RayTracing",
		"GSplats",
		"minmap",
	}

	dependson { 
		"EngineBackend", 
		"EngineUtils",
		"EngineSystems",
		"RayTracing",
		"GSplats",
	}

	debugenvs {
        "PATH=" .. assimp_bin_dir .. ";$(PATH)"
    }

	if vulkan_sdk ~= nil then
        includedirs {
            vulkan_sdk .. "/Include"
        }

        libdirs {
            vulkan_sdk .. "/Lib"
        }

        links {
            "vulkan-1.lib"
        }

        defines {
            "USE_VULKAN"
        }
    end

	filter "system:windows"
		systemversion "latest"
		defines { "_CRT_SECURE_NO_WARNINGS", "GLOG_NO_ABBREVIATED_SEVERITIES" }
		buildoptions { "/permissive-" }  -- <-- important

    filter "configurations:Debug"
        defines "APP_DEBUG"
        symbols "on"
        runtime "Debug"
        links { "assimp-vc143-mtd.lib" }

    filter "configurations:Release"
        defines "APP_RELEASE"
        optimize "on"
        runtime "Release"
        links { "assimp-vc143-mt.lib" }

	filter "configurations:Dist"
		defines "APP_DIST"
		optimize "on"
		runtime "Release"