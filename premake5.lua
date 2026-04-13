-- premake5.lua – WinTalker 64-bit Linux build
-- Run: premake5 gmake2   (or gmake)
--      make config=release

workspace "WinTalker"
    configurations { "Debug", "Release" }
    platforms      { "x64" }
    location       "./"

project "wintalker"
    kind        "ConsoleApp"
    language    "C"
    cdialect    "C99"
    targetname  "wintalker"

    -- Output directories
    objdir      "obj/%{cfg.buildcfg}"
    targetdir   "bin/%{cfg.buildcfg}"

    -- Include path
    includedirs { "include" }

    -- Source files (Linux platform layer + engine)
    files {
        "src/main.c",
        "src/Linux.c",
        "src/Engine.c",
        "src/BackEnd.c",
        "src/FrontEnd.c",
        "src/formantSynth.c",
        "src/Say.c",
        "src/Morph.c",
        "src/EngToP.c",
        "src/EmbeddedCmd.c",
        "src/Data.c",
        "src/Sounds.c",
        "src/english_lex.c",
    }

    -- Exclude the original Windows-only files
    removefiles {
        "src/Windows95.c",
        "src/Wavinout.c",
    }

    -- Preprocessor defines
    defines {
        "POWERPC_NATIVE_MT3=1",
        "_LINUX",
    }

    -- Compiler warnings / options
    warnings    "Default"

    -- Release configuration
    filter "configurations:Release"
        defines  { "NDEBUG" }
        optimize "Speed"

    -- Debug configuration
    filter "configurations:Debug"
        defines  { "DEBUG=1" }
        optimize "Off"
        symbols  "On"

    -- Linux link flags
    filter "system:linux"
        links   { "m" }
        buildoptions { "-Wall", "-Wno-unused-function" }
