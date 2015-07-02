function postBuildCommandsWin32()
	local solutionDirectory = ".."
	local binDirectory = string.gsub(targetdir(), "/", "\\")
	postbuildcommands { "copy /Y local\\windows\\lib\\*.dll " .. binDirectory }
end

solution "qhull"

	configurations { "Debug", "Release" }
	language "C++"
	includedirs { "." }
	flags { "NoMinimalRebuild", "EnableSSE", "EnableSSE2" }

	-- OS specific initializations
	local targetName;
	if os.is("macosx") then
		targetName = "osx"
		platforms {"x64"}
	else
		platforms {"x32", "x64"}
	end

	if os.is("windows") then
		targetName = "win"
		defines{ "WIN32" }
		if _ACTION == "vs2012" then
			buildoptions { "/MP "  } --multiprocessor build
			buildoptions { "/wd4800 /wd4146 /wd4244"  }
			defines { "_CRT_SECURE_NO_WARNINGS" }
			configuration {"Release"}
		end
	end

	-- Configuration specific definitions
	configuration "Debug"
		defines { "_DEBUG" }
		flags { "Symbols" }
	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }

	configuration {"x32", "Debug"}
		targetsuffix "D"
		targetdir "Bin/Debug/x86"
	configuration {"x64", "Debug"}
		targetsuffix "64D"
		targetdir "Bin/Debug/x64"
	configuration {"x32", "Release"}
		targetdir "Bin/Release/x86"
	configuration {"x64", "Release"}
		targetsuffix "64"
		targetdir "Bin/Release/x64"

	configuration {} -- Back to all configurations

	-- Project
	project "qhull"

	kind "ConsoleApp"
	defines { "GK_OPENGL4", "VERBOSE" }
	files { "*.h", "*.hpp", "*.cpp",
		"gKit/*.cpp", "gKit/*.h",
		"gKit/GL/*.cpp", "gKit/GL/*.h",
		"gKit/Widgets/*.cpp", "gKit/Widgets/*.h"
	}
	includedirs {
		".",
		"gKit",
		"local/windows/include"
	}

	configuration {"x32", "Debug"}
		targetdir "Bin/Debug/x86"
		postBuildCommandsWin32()
	configuration {"x64", "Debug"}
		targetdir "Bin/Debug/x64"
		postBuildCommandsWin32()
	configuration {"x32", "Release"}
		targetdir "Bin/Release/x86"
		postBuildCommandsWin32()
	configuration {"x64", "Release"}
		targetdir "Bin/Release/x64"
		postBuildCommandsWin32()
	configuration {}

	if os.is("windows") then
		defines { "WIN32", "NVWIDGETS_EXPORTS", "_USE_MATH_DEFINES", "_CRT_SECURE_NO_WARNINGS" }
		defines { "NOMINMAX" } -- allow std::min() and std::max() in vc++ :(((
		libdirs { "local/windows/lib" }
		links { "opengl32", "glew", "SDL2", "SDL2main", "SDL2_image", "SDL2_ttf" }
	end

if _ACTION == "clean" then
	os.rmdir("Bin");
	
	os.remove("qhull.sdf");
	os.remove("qhull.v12.suo");
end