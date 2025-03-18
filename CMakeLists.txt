cmake_minimum_required(VERSION 3.8)
project("cssHack")


if(CMAKE_SYSTEM_NAME MATCHES Windows)
else()
    # force 32bit compilation
    # source: https://stackoverflow.com/questions/5805874/the-proper-way-of-forcing-a-32-bit-compile-using-cmake
    set(CMAKE_C_FLAGS -m32)
    set(CMAKE_CXX_FLAGS -m32)

endif()

###########################
# shared library target
###########################
add_library(cssHack SHARED
    src/Log.cpp
    src/Aimbot.cpp
    src/Bunnyhop.cpp
    src/Detour.cpp
    src/DetourToCallback.cpp
    src/Hack.cpp
    src/MemoryUtils.cpp
    src/libEntry.cpp
    src/Utility.cpp
    src/Visuals/DrawHook.cpp
    src/Visuals/ESP.cpp
    #src/Visuals/Wallhack.cpp
    src/Input.cpp
    src/Visuals/GUI.cpp
    src/Visuals/Gui_imgui_impl_sdl.cpp
    src/Pointers/GameVars.cpp
    src/Pointers/Signatures.cpp
)

target_include_directories(cssHack
    PRIVATE src
)

# OS-dependendent implementation
if(CMAKE_SYSTEM_NAME MATCHES Windows)
    target_sources(cssHack
                   PRIVATE src/MemoryUtils_Win.cpp
    )
else()
    target_sources(cssHack
                   PRIVATE src/MemoryUtils_Linux.cpp
    )
endif()

###########################
# dependencies
###########################
add_subdirectory(lib/BoyerMoore-DontCare)

set(IMGUI_SOURCES
    lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/examples/imgui_impl_opengl3.cpp
    lib/imgui/examples/libs/gl3w/GL/gl3w.c
)

set(IMGUI_INCLUDE_DIRS
    lib/imgui
    lib/imgui/examples
    lib/imgui/examples/libs/gl3w
)

set(GLM_INCLUDE_DIRS
    lib/glm
)

# SDL2
# source: https://trenki2.github.io/blog/2017/06/02/using-sdl2-with-cmake/
# platform check
# source: https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-To-Write-Platform-Checks
if(CMAKE_SYSTEM_NAME MATCHES Windows)
    set(SDL2_DIR "lib/windows/SDL2-devel-2.0.10-VC/SDL2-2.0.10")
    find_package(SDL2 REQUIRED)
else()
    # enable setting variables for the subdirectory
    # source: https://stackoverflow.com/a/66342383
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

    set(SDL_X11 OFF)
    set(SDL_STATIC OFF)

    # thanks to saxbophone
    # source: https://github.com/libsdl-org/SDL/issues/2010#issuecomment-1903440780
    add_subdirectory(lib/SDL EXCLUDE_FROM_ALL)

    # don't postfix build name with "d" in Debug builds so the built lib name matches the SDL lib loaded in the game
    set_target_properties(SDL2 PROPERTIES DEBUG_POSTFIX "")

endif()
find_package(Threads REQUIRED)

###########################
# dependencies - add to target
###########################
target_sources(cssHack
    PRIVATE ${IMGUI_SOURCES}
    PRIVATE ${BOYERMOOREDONTCARE_SOURCES}
)

target_include_directories(cssHack
    PRIVATE ${IMGUI_INCLUDE_DIRS}
    PRIVATE ${BOYERMOOREDONTCARE_INCLUDE_DIRS}
    PRIVATE ${GLM_INCLUDE_DIRS}
)

target_link_libraries(cssHack
    PRIVATE ${CMAKE_THREAD_LIBS_INIT}
    SDL2
)

# link OpenGL
if(CMAKE_SYSTEM_NAME MATCHES Windows)
    #set(OpenGL_GL_PREFERENCE LEGACY)
    find_package(OpenGL REQUIRED)
    target_link_libraries(cssHack
        PRIVATE ${OPENGL_LIBRARIES}
    )
    target_include_directories(cssHack
        PRIVATE ${OPENGL_INCLUDE_DIRS}
    )
else()
    target_link_libraries(cssHack
        PRIVATE "/usr/lib/i386-linux-gnu/libGL.so.1"
    )
endif()

###########################
# compile / link flags
###########################
target_compile_features(cssHack PRIVATE cxx_std_20)

target_compile_options(cssHack
    PRIVATE -Wextra
    PRIVATE -Wpedantic
)

# hidden symbols in release-builds
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    set_target_properties(cssHack PROPERTIES
        CXX_VISIBILITY_PRESET hidden
    )
endif()

if(CMAKE_SYSTEM_NAME MATCHES Windows)
    # disable warning caused in "lib/windows/SDL2-devel-2.0.10-VC/SDL2-2.0.10/include/SDL_config.h"
    target_compile_options(cssHack
        PRIVATE -Wno-language-extension-token
    )
endif()