#!/usr/bin/env bash

libraryPath=$(realpath "../cmake-build-debug/libcssHack.so")
grep $libraryPath "/proc/$(pidof hl2_linux)/maps"