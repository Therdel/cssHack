#!/usr/bin/env bash

# cd into the script's directory
cd "$(dirname "$0")"

# configure
mkdir -p ../build
cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++ --no-warn-unused-cli -S.. -B../build

# build
cmake --build ../build --config Debug --target all -j --
