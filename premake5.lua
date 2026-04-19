-- premake5.lua – WinTalker 64-bit Linux build
-- Run: premake5 gmake2   (or gmake)
--      make config=release

workspace "WinTalker"
    configurations { "Debug", "Release" }
    platforms      { "x64" }
    location       "."

project "wintalker"
    kind      "ConsoleApp"
    language  "C"
    targetdir "bin/%{cfg.buildcfg}"
    objdir    "obj/%{cfg.buildcfg}"

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
        "src/Singing.c",
        "src/Data.c",
        "src/Sounds.c",
        "src/english_lex.c",
    }

    -- Exclude the original Windows-only files
    removefiles {
        "src/Windows95.c",
        "src/Wavinout.c",
    }

    -- Linux build settings
    filter "system:linux"
        architecture "x64"
        staticruntime "Off"

    -- Configurations
    filter "configurations:Release"
        defines  { "RELEASE=1" }
        optimize "On"

    filter "configurations:Debug"
        defines  { "DEBUG=1" }
        optimize "Off"
        symbols  "On"

    -- Linux link flags
    filter "system:linux"
        links   { "m" }
        buildoptions { "-Wall", "-Wno-unused-function" }
