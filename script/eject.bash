#!/usr/bin/env bash
# source: https://aixxe.net/2016/09/shared-library-injection

process=hl2_linux
pid=$(pidof $process)
libraryPath=$(realpath "../build/libcssHack.so")
library=$(basename $libraryPath)

# check running
if [ -z "$pid" ]
then
    echo "$process is not running"
    exit 1
fi

echo "Process: $process (pid: $pid)"
echo "Library: $libraryPath"

# check if library is injected
if ! grep -q $libraryPath /proc/$pid/maps ;
then
    echo "Library $library is not loaded. Exiting."
    exit
fi

# eject
echo "Library $library is loaded. Ejecting"

# enable gdb attach with ptrace (disable ptrace scope)
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope

# write gdb script
echo "
attach $pid
set \$dlopen = (void*(*)(char*, int)) dlopen
set \$dlclose = (int(*)(void*)) dlclose
set \$library = \$dlopen(\"$libraryPath\", 6)
call \$dlclose(\$library)
call \$dlclose(\$library)
detach
quit
" > eject.gdb

# eject
gdb -q --batch --command=eject.gdb

# remove gdb script
rm eject.gdb

# check running
pid=$(pidof $process)
if [ -z "$pid" ]
then
    echo "Ejection failed: $process crashed"
    exit 1
fi

# check success
if ! grep -q $libraryPath /proc/$pid/maps ; then
    echo "Ejected library $library from process $process (pid: $pid)"
else
    echo "Ejection failed: Library $library still in process $process (pid: $pid)"
fi