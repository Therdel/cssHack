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

# check already injected
if grep --quiet $libraryPath /proc/$pid/maps ; then
    echo -e "\e[31mLibrary $library already loaded. Exiting.\e[0m"
    exit 1
fi

# inject
echo -e "\e[33mInjecting library $library.\e[0m"

# write gdb script
script="
# Enable non-stop mode and async execution
# set target-async on
# set non-stop on
# set auto-solib-add off

set pagination off

set print thread-events off
attach $pid
set print thread-events on
# cont

set \$dlopen = (void*(*)(char*, int)) dlopen
set \$result = \$dlopen(\"$libraryPath\", 1)

if \$result == 0
    printf \"Injection Error: %s\\n\", (char*)dlerror()
else
    print \"Injection Success\"
end

detach
quit
"
script_file=$(mktemp)
echo "$script" > "$script_file"

# inject
# run GDB as root for CAP_SYS_PTRACE,
# in order to attach to processes owned by other users
sudo gdb -n --batch --command="$script_file"

rm "$script_file"

# check running
if [ -z "$pid" ]; then
    echo -e "\e[31mInjection failed: Target process crashed\e[0m"
    exit 1
fi

# check success
if grep --quiet "$libraryPath" "/proc/$pid/maps"; then
    echo -e "\e[32mInjection successful\e[0m"
else
    echo -e "\e[31mInjection failed: Library not found in target process\e[0m"
fi
