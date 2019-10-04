cmake_minimum_required(VERSION 3.8)
project("cssHack")
set(CMAKE_CXX_STANDARD 17)

# compiler warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# set debug flags for debug build
set(CMAKE_CXX_FLAGS_DEBUG "-g")
# enable optimization for relase build
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    ADD_DEFINITIONS( -DCMAKE_DEBUG)
endif()

# external libraries
find_package(Threads REQUIRED)

set(HACK_SOURCES
        src/Log.cpp
        src/Aimbot.cpp
        src/Bunnyhop.cpp
        src/Hack.cpp
        src/MemoryUtils.cpp
        src/libEntry.cpp
        src/Utility.cpp
        src/Visuals/DrawHook.cpp
        src/Visuals/ESP.cpp
        src/Visuals/Wallhack.cpp
        src/Input.cpp
        src/Visuals/GUI.cpp
        src/Visuals/Gui_imgui_impl_sdl.cpp
        src/Pointers/GamePointerDef.cpp
        src/Pointers/GamePointerUpdater.cpp)

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


add_library(cssHack SHARED
        ${HACK_SOURCES}
        ${IMGUI_SOURCES}
        )

# source: https://trenki2.github.io/blog/2017/06/02/using-sdl2-with-cmake/
find_package(SDL2 REQUIRED)
#set(OpenGL_GL_PREFERENCE LEGACY)
#find_package(OpenGL REQUIRED)
target_include_directories(cssHack
        PRIVATE ${IMGUI_INCLUDE_DIRS}
        PRIVATE ${GLM_INCLUDE_DIRS}
        PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(cssHack
        PRIVATE ${CMAKE_THREAD_LIBS_INIT}
        PRIVATE "/usr/lib/i386-linux-gnu/libSDL2-2.0.so.0"
        PRIVATE "/usr/lib/i386-linux-gnu/libGL.so.1"
        )

set_target_properties(cssHack PROPERTIES
        # use intel syntax for inline assembly
        # source: https://stackoverflow.com/questions/10990018/how-to-generate-assembly-code-with-clang-in-intel-syntaxhttps://stackoverflow.com/questions/10990018/how-to-generate-assembly-code-with-clang-in-intel-syntax
        COMPILE_FLAGS "-masm=intel"
        # force 32bit compilation
        # source: https://stackoverflow.com/questions/5805874/the-proper-way-of-forcing-a-32-bit-compile-using-cmake
        COMPILE_OPTIONS "-m32"
        LINK_OPTIONS "-m32"
        # default to hidden symbols
        CXX_VISIBILITY_PRESET hidden
        )
