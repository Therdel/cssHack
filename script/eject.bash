#!/usr/bin/env bash
# source: https://aixxe.net/2016/09/shared-library-injection

# cd into the script's directory
cd "$(dirname "$0")"

process=hl2_linux
pid=$(pidof $process)
libraryPath=$(realpath "../build/libcssHack.so")
library=$(basename $libraryPath)

# check running
if [ -z "$pid" ]; then
    echo -e "\e[31mProcess $process is not running\e[0m"
    exit 1
fi

echo -e "\e[33mProcess: $process (pid: $pid)\e[0m"
echo -e "\e[33mLibrary: $libraryPath\e[0m"

# check if library is injected
if ! grep -q $libraryPath /proc/$pid/maps ; then
    echo -e "\e[31mLibrary $library is not loaded. Exiting.\e[0m"
    exit 1
fi

# eject
echo -e "\e[33mLibrary $library is loaded. Ejecting\e[0m"

# write gdb script
script="
set print thread-events off
set auto-solib-add off
attach $pid
set print thread-events on

set \$dlopen = (void*(*)(char*, int)) dlopen
set \$dlclose = (int(*)(void*)) dlclose
# create our own thread so as not to block the currently hijacked game thread,
# which we may need to run during dlclose() in case it's currently running
# through any of our hooks.
set \$pthread_create = (int(*)(void*, void*, void*, void*)) pthread_create
set \$malloc = (void*(*)(int)) malloc

# Allocate 8 bytes of heap memory in the target process 
# to hold the new thread's ID safely without smashing the stack
set \$tid_ptr = \$malloc(8)

# get current handle (Refcount ticks up to 2)
set \$library = \$dlopen(\"$libraryPath\", 5)

call \$dlclose(\$library)
call \$pthread_create(\$tid_ptr, 0, \$dlclose, \$library)

detach
quit
"
script_file=$(mktemp)
echo "$script" > "$script_file"

# eject
# run GDB as root for CAP_SYS_PTRACE,
# in order to attach to processes owned by other users
sudo gdb -n --batch --command="$script_file"

rm "$script_file"

echo -e "\e[33mWaiting for background thread to complete ejection...\e[0m"
TIMEOUT_SECONDS=5
SLEEP_INTERVAL=0.1
TIME_ELAPSED=0
while grep -q "$libraryPath" /proc/$pid/maps; do
    sleep $SLEEP_INTERVAL
    TIME_ELAPSED=$(echo "$TIME_ELAPSED + $SLEEP_INTERVAL" | bc)
    
    if (( $(echo "$TIME_ELAPSED >= 5" | bc -l) )); then
        echo -e "\e[31mEjection timed out! The library is still in memory (Deadlock?).\e[0m"
        exit 1
    fi
done

# check running
if [ -z "$pid" ]; then
    echo -e "\e[31mEjection failed: Target process crashed\e[0m"
    exit 1
fi

# check success
if ! grep --quiet "$libraryPath" "/proc/$pid/maps"; then
    echo -e "\e[32mEjection successful\e[0m"
else
    echo -e "\e[31mEjection failed: Library still in target process\e[0m"
fi