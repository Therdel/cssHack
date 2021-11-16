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

