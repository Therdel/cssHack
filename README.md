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
    install packages for 32bit compilation (*game is 32bit only*)
    ```bash
    sudo apt-get install gcc-multilib g++-multilib
    ```
1. **[only for GNU/Linux]**<br>
    install 32bit [libSDL2](https://wiki.libsdl.org/SDL2/Installation#linuxunix) - *used for hooking renderer & IO*
    ```bash
    sudo dpkg --add-architecture i386   # enable adding 32bit packages
    sudo add-apt-repository universe    # add 'universe' package repo
    sudo apt update
    sudo apt-get install libsdl2-dev:i386
    ```
1. clone repository
    ```bash
    git clone https://github.com/Therdel/cssHack.git --recurse-submodules
    cd cssHack
    ```
1. build
    ```bash
    mkdir build && cd build
    cmake .. && make -j
    ```

### Injecting
#### Windows
*Winject 1.7* is easy to use. **use at your own risk**. Get it from e.g. [oldschoolhack.me](https://www.oldschoolhack.me/en/downloads/tools/3610-winject-17)
#### GNU/Linux
- I bundled three scripts in the ```scripts/``` directory: *inject.bash*, *eject.bash* & *reinject.bash*. I adapted [aixxe](https://aixxe.net/2016/09/shared-library-injection)'s idea for these.
- You may have to adapt paths in the scripts.
- These attach the GDB Debugger to the game process for injection. Doing this to a process we haven't started isn't allowed under normal circumstances. So execute ```scripts/disable_ptrace_scope.bash``` with sudo privileges once per login session to use these scripts.