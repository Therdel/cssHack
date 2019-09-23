#!/usr/bin/env bash

libraryPath=$(realpath "../cmake-build-debug/libcssHack.so")
cssPath="$HOME/.steam/steamapps/common/Counter-Strike Source"
export LD_PRELOAD=$libraryPath

#cd "$cssPath"
exec "$cssPath""/hl2.sh" -game cstrike -sw -novid -console -nobreakpad +sv_lan 1 -map aim_ag_texture2.bsp

