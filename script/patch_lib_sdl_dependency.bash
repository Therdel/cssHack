#!/usr/bin/env bash

# cd into the script's directory
cd "$(dirname "$0")"

patchelf --add-needed libSDL2-2.0.so.0 ../build/libcssHack.so