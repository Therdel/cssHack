#!/usr/bin/env bash
# source: https://aixxe.net/2016/12/internal-dev-environment

GAMEROOT=/home/theo/.local/share/Steam/steamapps/common/"Counter-Strike\ Source"/
export LD_LIBRARY_PATH="${GAMEROOT}"/bin:$LD_LIBRARY_PATH
GAMEEXE=hl2_linux
ulimit -n 2048
export __GL_THREADED_OPTIMIZATIONS=1


#cd "$GAMEROOT"
"/home/theo/.local/share/Steam/steamapps/common/Counter-Strike\ Source/hl2_linux" -game cstrike -sw -novid -console -nobreakpad +sv_lan 1 -map aim_ag_texture2.bsp


"${GAMEROOT}"/hl2_linux -game cstrike -sw -novid -console -nobreakpad +sv_lan 1

"${GAMEROOT}"/hl2.sh -game cstrike -sw -novid -console -nobreakpad +sv_lan 1
