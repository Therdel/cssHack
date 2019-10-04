#!/usr/bin/env bash
# source: https://aixxe.net/2016/09/shared-library-injection

process=hl2_linux
pid=$(pidof $process)
libraryPath=$(realpath "../cmake-build-debug/libcssHack.so")
library=$(basename $libraryPath)

# check running
if [ -z "$pid" ]
then
    echo "$process is not running"
    exit 1
fi

echo "Process: $process (pid: $pid)"
echo "Library: $libraryPath"

# check already injected
if grep -q $libraryPath /proc/$pid/maps; then
    echo "$library already loaded"
    exit
fi

# write gdb script
echo "attach $pid
set \$dlopen = (void*(*)(char*, int)) __libc_dlopen_mode
set \$result = \$dlopen(\"$libraryPath\", 1)
if \$result == 0
printf \"Error: %s\\n\", (char*)dlerror()
else
print \"Success\"
end
detach
quit
" > inject.gdb

# inject
gdb -q --batch --command=inject.gdb

rm inject.gdb

# check running
pid=$(pidof $process)
if [ -z "$pid" ]
then
    echo "Injection failed: $process crashed"
    exit 1
fi

# check success
if grep -q $libraryPath /proc/$pid/maps; then
    echo "Injected $library into $process ($pid)"
else
    echo "Injection failed: library not found in process space"
fi

