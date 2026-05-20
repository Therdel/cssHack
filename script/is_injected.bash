#!/usr/bin/env bash

# cd into the script's directory
cd "$(dirname "$0")"

process=hl2_linux
pid=$(pidof $process)
libraryPath=$(realpath "../build/libcssHack.so")
library=$(basename $libraryPath)

# check running
if [ -z "$pid" ]; then
    echo -e "\e[33mProcess $process is not running\e[0m"
    exit 1
fi

# check if library is injected
if ! grep -q $libraryPath /proc/$pid/maps ; then
    echo -e "\e[31mLibrary $library is NOT loaded\e[0m"
    exit 1
else
    echo -e "\e[32mLibrary $library is loaded\e[0m"
    exit 0
fi
