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

# check already injected
if grep -q $libraryPath /proc/$pid/maps ;
then
    echo "Library $library already loaded. Exiting."
    exit
fi

# inject
echo "Injecting library $library."

# write gdb script
echo "
attach $pid
set \$dlopen = (void*(*)(char*, int)) dlopen
set \$result = \$dlopen(\"$libraryPath\", 1)
if \$result == 0
printf \"Injection Error: %s\\n\", (char*)dlerror()
else
print \"Injection Success\"
end
detach
quit
" > inject.gdb

# inject
gdb -q --batch --command=inject.gdb

# remove gdb script
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
    echo "Injected library $library into process $process (pid: $pid)"
else
    echo "Injection failed: Library $library not found in process $process (pid: $pid)"
fi