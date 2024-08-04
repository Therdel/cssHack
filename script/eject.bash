#!/usr/bin/env bash
# source: https://aixxe.net/2016/09/shared-library-injection

process=hl2_linux
pid=$(pidof $process)
libraryPath=$(realpath "../build/libcssHack.so")
library=$(basename $libraryPath)

echo "Process: $process (pid: $(pidof $process))"
echo "Library: $library"

if grep -q $libraryPath /proc/$pid/maps ; then
    echo "$library is loaded. Ejecting"
    sudo gdb -n -q -batch \
      -ex "attach $pid" \
      -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
      -ex "set \$dlclose = (int(*)(void*)) dlclose" \
      -ex "set \$library = \$dlopen(\"$libraryPath\", 6)" \
      -ex "call \$dlclose(\$library)" \
      -ex "call \$dlclose(\$library)" \
      -ex "detach" \
      -ex "quit"

  # check running
  pid=$(pidof $process)
  if [ -z "$pid" ]
  then
      echo "Ejection failed: $process crashed"
      exit 1
  fi

  # check success
  if grep -q $libraryPath /proc/$pid/maps; then
      echo "Ejection failed: library still in process space"
  else
      echo "Ejected $library from $process ($pid)"
  fi
else
    echo "$library is not loaded"
fi