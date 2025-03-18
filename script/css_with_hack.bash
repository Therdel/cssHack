#!/usr/bin/env bash

# cd into the script's directory
cd "$(dirname "$0")"

# libraryPath=$(realpath "../build/libcssHack.so")
# export LD_PRELOAD=$libraryPath

exec /home/theo/.local/share/Steam/steamapps/common/"Counter-Strike Source"/hl2.sh -steam -game cstrike -game cstrike -novid -console -nobreakpad +sv_lan 1 -map aim_ag_texture2.bsp -sw 

