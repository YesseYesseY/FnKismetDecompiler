workspace "KismetDecompiler"
    configurations { "Release" }

project "KismetDecompiler"
    kind "SharedLib"
    language "C++"
    cppdialect "C++23"
    targetdir "bin"
    objdir "obj"
    files {
        "Src/*.cpp",
    }
    -- includedirs {
    -- }
    buildoptions { "/wd4369", "/wd4309" }
    architecture "x64"
