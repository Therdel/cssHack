#!/usr/bin/env bash
# source: https://aixxe.net/2016/09/shared-library-injection

process=hl2_linux
pid=$(pidof $process)
libraryPath=$(realpath "../cmake-build-debug/libcssHack.so")
library=$(basename $libraryPath)

echo "Process: $process (pid: $(pidof $process))"
echo "Library: $libraryPath"

if grep -q $libraryPath /proc/$pid/maps; then
    echo "$library already loaded"
    exit
fi

sudo gdb -n -q -batch \
    -ex "attach $pid" \
    -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
    -ex "call \$dlopen(\"$libraryPath\", 1)" \
    -ex "detach" \
    -ex "quit"

echo "Injected $library into $process ($pid)"
