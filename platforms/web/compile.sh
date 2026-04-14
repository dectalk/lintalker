#!/bin/bash
# WinTalker – Emscripten build
# Usage: ./compile.sh [--debug]
# Then serve with:  python3 -m http.server  (or emrun index.html)

set -e
source ~/.bash_profile

cd "$(dirname "$0")"

SRCS=(
    ../../src/Engine.c
    ../../src/BackEnd.c
    ../../src/FrontEnd.c
    ../../src/formantSynth.c
    ../../src/Say.c
    ../../src/Morph.c
    ../../src/EngToP.c
    ../../src/EmbeddedCmd.c
    ../../src/Data.c
    ../../src/Sounds.c
    ../../src/english_lex.c
    ../../src/Linux.c
    Web.c
)

DEFINES=(
    -DPOWERPC_NATIVE_MT3=1
    -DNO_FILESYSTEM=1
)

if [[ "$1" == "--debug" ]]; then
    OPT="-O0 -g"
    echo "Debug build"
else
    OPT="-O2"
    echo "Release build"
fi

emcc "${SRCS[@]}" \
    -I ../../include \
    "${DEFINES[@]}" \
    $OPT \
    -s MODULARIZE=1 \
    -s EXPORT_NAME=WinTalker \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORTED_FUNCTIONS='["_malloc","_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8","HEAP16"]' \
    -lm \
    -o wintalker.js

echo "Done → wintalker.js + wintalker.wasm"
