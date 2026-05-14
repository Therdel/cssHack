# Cross Platform Counter-Strike: Source hack
##### for GNU/Linux and Windows  


https://user-images.githubusercontent.com/14974231/142050572-a557c6a9-42d9-40d3-9584-dd1b008f7f88.mp4



### Features
- Bunnyhop
- Aimbot
    - **360 NoScope Aimbot**
    - Aim by FOV / Distance
    - Aimkey
    - Autoshoot
    - No Recoil
    - No Visual Recoil
- Triggerbot
    - **360 NoScope Triggerbot**
- Visuals
    - *Real* Crosshair / Effective Bullet Angles
    - Draw effective FOV
    - Player Position/Orientation
- Ingame Cheat Menu
- Panic Key (disable everything on button press)
    
### Media
- FOV Aimbot
- [360 Aimbot](media/360_hack.mp4)
- Bunnyhop
- Player Position/Orientation
- NoRecoil
- Bullet ESP

### Techniques used
- **Code injection**
    - Windows: CreateRemoteThread/LoadLibrary Method
    - GNU/Linux: GDB attach/dlopen script
- **Detouring / Hooking**
    - Detour to thiscall / generic lambda
    - Trampolines / Thunking
    - Mid function / jmp detour
    - Call redirection
- **Auto offsets** (Game update resistance)
    - [Signature Scanning](https://wiki.alliedmods.net/Signature_Scanning) to stay functional after game binary changes    
    (Own [variation](https://github.com/Therdel/BoyerMoore-DontCare) of BoyerMoore)
    

### Credits
- [aixxe](https://aixxe.net/2016/09/shared-library-injection) - GNU/Linux SO injection

## Getting Started
### Building
1. **[only for GNU/Linux]**<br>
    install compiler & tools
    ```bash
    sudo apt-get install cmake ninja clang clang-tools
    ```
    install packages for 32bit compilation (*game is 32bit only*)
    ```bash
    sudo apt-get install gcc-multilib g++-multilib
    ```
    install OpenGL dev lib *used for rendering*
    ```bash
    sudo apt-get install libgl-dev libxext-dev libudev-dev libxkbcommon-dev
    ```
    **[only for Windows]**<br>
    install compiler, tools & Windows SDK
    ```bash
    winget install --id=Kitware.CMake -e
    winget install --id=Ninja-build.Ninja -e
    winget install -i LLVM.LLVM
    TODO TEST (doesn't work, no package matching input criteria) winget install -e --id Microsoft.WindowsSDK
    ```
    install latest Windows SDK by installing `Visual Studio`
2. clone repository
    ```bash
    git clone https://github.com/Therdel/cssHack.git --recurse-submodules --shallow-submodules --depth=1
    cd cssHack
    ```
3. build
    ```bash
    # configure build
    cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++ --no-warn-unused-cli -S . -B ./build -G Ninja

    # build
    cmake --build ./build --config Debug --target all --parallel
    ```

### Injecting **use at your own risk**
#### Windows
*Winject 1.7* is easy to use **use at your own risk**. Get it from e.g. [oldschoolhack.me](https://www.oldschoolhack.me/en/downloads/tools/3610-winject-17)
#### GNU/Linux
- I bundled three scripts in the ```scripts/``` directory: *inject.bash*, *eject.bash* & *reinject.bash*. I adapted [aixxe](https://aixxe.net/2016/09/shared-library-injection)'s idea for these.
- You may have to adapt paths in the scripts.
- These attach the GDB Debugger to the game process for injection. Doing this to a process we haven't started isn't allowed under normal circumstances. So execute ```scripts/disable_ptrace_scope.bash``` with sudo privileges once per login session to use these scripts.

### Development & Debugging
VSCode extensions (see [.vscode/extensions.json](.vscode/extensions.json)):
- [C/C++ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack)
- [CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) debug using LLDB, as lib is built using Clang(LLVM)

### Counter-Strike: Source version
- this hack was made for the 32bit version and thus broke with the [64bit update in Feb 2025](https://steamdb.info/patchnotes/17399420/)
- you can try it out locally by downgrading to [Build 17399420](https://steamdb.info/patchnotes/6946501/), the last 32bit version
- see [this guide](https://steamcommunity.com/sharedfiles/filedetails/?id=889624474) to downgrade with the Steam console  
- see Linux version manifest in [./game_cfg/appmanifest_240.acf](game_cfg/appmanifest_240.acf)

### Working Cheat-Engine Version
Cheat-Engine 7.1

## TODO
- [ ] Fix Panic Key (now: works only directly after injecting)
- [ ] bump all cmakelists versions 
- [ ] windows SDK without visual studio install
- [ ] windows manifest
- [ ] downgrading tutorial (from up-to-date to linux + windows)
- isCrouching (better: BoneAim)
- don't aim at kicked bots at (0,0,0)
- Autopistol
  - using s_client_localplayer_shotsfired - if wearing a pistol, let go if it turns 1
- BSP Parsing
  - implementation
    - [Valve BSP docs](https://www.unknowncheats.me/forum/counterstrike-global-offensive/136369-bsp-parsing.html)
    - [algorithm](https://www.unknowncheats.me/forum/counterstrike-global-offensive/136369-bsp-parsing.html)
    - [source SDK 2013 Parser?](https://github.com/Therdel/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/public/gamebspfile.h)
    - [source SDK 2013 Raytracer?](https://github.com/Therdel/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/public/raytrace.h)
    - [ReactiioN1337/valve-bsp-parser](https://github.com/ReactiioN1337/valve-bsp-parser)
    - access Bones: [CBoneAccessor](https://github.com/Therdel/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/public/bone_accessor.h#L20)
    - CSS-external cheat with BSP Parsing [source](https://github.com/ALittlePatate/CSS-external)
  - Features
    - Aimbot: Visible only
    - Aimbot: Prioritize Enemies seeing me (90° FOV cone trace)/ aiming at me (narrower cone trace)
    - ESP: Visibility
    - ESP: Sees me
    - ESP: Aims at me
    - ESP: HEAD Emoji
- Bone Matrix
  - implementation
    - manually
        1. get position of player head bone via footpos+viewHeight+-5
        2. find bonematrix candidate(s) base ptr
        4. repeat for a bot with bot_mimic - but have them on a different height and use their radar foot pos
        5. find that bonematrix candiate(s) base ptr
        6. do pointer scan for both
        7. both pointers must rely in a similar location - the entity list
    - [?location](https://www.google.com/url?sa=t&source=web&rct=j&opi=89978449&url=https://www.youtube.com/watch%3Fv%3DelKUMiqitxY&ved=2ahUKEwiBsc6X55qQAxX8SPEDHTpuEhIQ3aoNegQIAxAC&usg=AOvVaw3MO61Z5QqIcPg7CJRCN9nl)
    - Use source SDK, via [CreateInterface](https://www.unknowncheats.me/forum/4149974-post321.html)
  - position:
      2287AA2C maybe_bonelist
      23533A9C probably_bonelist
        23533800 bonelist_beg
      2353415C short_bonelist_3x4x9
      231EF568 > bonlist_beg
      231EF800 > bonlist_beg
      231EF810 > bonlist_beg

      231EF0E0 localplayer_viewoffsetZ
      231EF258 localplayer_pos_feet
  - Features
    - All-bone aim (max dmg)
    - ESP: Skeleton
    - ESP: Correct Box ESP
- Use Source SDK (also see Bone Matrix, duplicate info)
  - [x] how to use CreateInterface to actually get interfaces?
    - use dlsym/`MemoryUtils::getSymbolAddress(libNames::client, "CreateInterface")`
    - source: `sp/src/public/tier1/interface.h`::`DLL_EXPORT void* CreateInterface(const char *pName, int *pReturnCode)`
    - [x] use IDA to look for CreateInterface symbol or smth
  - filter Players: [source0](https://www.unknowncheats.me/forum/4141444-post2.html)
    - `IClientEntityList::GetClientEntity(int)` for 0..64
    - check for null
    - `static_cast` to `C_BaseEntity`
    - check `bool C_BaseEntity::IsPlayer()`
    - get `C_BaseAnimating*` from `C_BaseAnimating* C_BaseEntity::GetBaseAnimating()`
      - `CBoneAccessor C_BaseAnimating::m_BoneAccessor` -> cast to derived class
      - `const matrix3x4_t& CBoneAccessor::GetBone( int iBone ) const`
      - ? ... `bool C_BaseAnimating::SetupBones( matrix3x4_t *pBoneToWorldOut, int nMaxBones, [..])`?
    - [ ] `IPlayerInfo *CBasePlayer::GetPlayerInfo()`
      - [ ] [IPlayerInfo](src/public/game/server/iplayerinfo.h)
        - `getHealth()`
        - `getName()`
        - `GetTeamIndex()`
        - `IsConnected()` (if the Player slot is valid)
        - `GetArmorValue()`
        - `GetWeaponName()`
        - `GetLastUserCommand()`

  - [IBaseClientDLL::CreateMove](https://developer.valvesoftware.com/wiki/Usercmd)
  - **Interesting Interfaces**
    - mp/src/public/engine/IEngineTrace.h::IEngineTrace
      - master: [IEngineTrace](src/public/engine/IEngineTrace.h)
    - mp/src/game/client/cliententitylist.cpp::IClientEntityList
      - master: [src/public/icliententitylist.h](IClientEntityList)
    - [IVDebugOverlay](src/public/engine/ivdebugoverlay.h)
      - [Youtube Demo CSGO](https://www.youtube.com/watch?v=PvEK--0DdhU)
    - `C_BaseEntity::TraceAttack()`
- Netvars code [frk1/hazedumper-rs](https://github.com/frk1/hazedumper-rs/blob/master/src/games/csgo/netvars.rs)