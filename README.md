# Cross Platform Counter-Strike: Source hack
##### for GNU/Linux and Windows  

https://user-images.githubusercontent.com/14974231/119541218-a66b7d00-bd8e-11eb-812d-baee80f0fc8d.mp4
https://github.com/Therdel/cssHack/blob/master/media/360_hack.mp4


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

