#!/usr/bin/env bash

# cd into the script's directory
cd "$(dirname "$0")"

libraryPath=$(realpath "../build/libcssHack.so")
grep $libraryPath "/proc/$(pidof hl2_linux)/maps"