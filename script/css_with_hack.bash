#!/usr/bin/env bash

# cd into the script's directory
cd "$(dirname "$0")"

libraryPath=$(realpath "../build/libcssHack.so")
cssPath="$HOME/.steam/steamapps/common/Counter-Strike Source"
export LD_PRELOAD=$libraryPath

#cd "$cssPath"
exec "$cssPath""/hl2.sh" -game cstrike -sw -novid -console -nobreakpad +sv_lan 1 -map aim_ag_texture2.bsp

