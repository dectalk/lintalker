idf_component_register(
    SRCS
        "src/BackEnd.c"
        "src/Data.c"
        "src/EmbeddedCmd.c"
        "src/Engine.c"
        "src/english_lex.c"
        "src/EngToP.c"
        "src/formantSynth.c"
        "src/FrontEnd.c"
        "src/Linux.c"
        "src/Morph.c"
        "src/Say.c"
        "src/Singing.c"
        "src/Sounds.c"
        "src/esp32_glue.c"
    INCLUDE_DIRS
        "include"
        "src"
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC NO_FILESYSTEM)

# Legacy code has many warnings that IDF treats as errors by default.
# We disable them here to allow it to compile without rewriting the engine.
target_compile_options(${COMPONENT_LIB} PRIVATE 
    "-Wno-multichar" 
    "-Wno-parentheses" 
    "-Wno-maybe-uninitialized"
    "-Wno-array-bounds"
    "-Wno-implicit-fallthrough"
    "-Wno-unused-variable"
    "-Wno-unused-but-set-variable"
    "-Wno-pointer-to-int-cast"
    "-Wno-int-to-pointer-cast"
    "-Wno-missing-braces"
    "-Wno-unknown-pragmas"
    "-Wno-comment"
    "-Wno-cast-function-type"
    "-Wno-unused-label"
    "-Wno-unused-function"
)
