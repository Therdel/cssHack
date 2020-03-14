cmake_minimum_required(VERSION 3.8)
project("cssHack")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
  # compiler warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
  # set debug flags for debug build
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# no c++98 incompatible messages
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat -Wno-c++98-compat-pedantic")
# enable optimization for relase build
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    ADD_DEFINITIONS( -DCMAKE_DEBUG)
endif()
        
###########################
# sources
###########################
set(HACK_SOURCES
        src/Log.cpp
        src/Aimbot.cpp
        src/Bunnyhop.cpp
        src/Detour.cpp
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
        src/Pointers/GamePointerUpdater.cpp
        src/Pointers/Signatures.cpp)

set(HACK_SOURCES_LINUX
        src/MemoryUtils_Linux.cpp
        )

set(HACK_SOURCES_WINDOWS
        src/MemoryUtils_Win.cpp
        )

if(CMAKE_SYSTEM_NAME MATCHES Windows)
    list(APPEND HACK_SOURCES ${HACK_SOURCES_WINDOWS})
else()
    list(APPEND HACK_SOURCES ${HACK_SOURCES_LINUX})
endif()

###########################
# external libraries
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
if(CMAKE_SYSTEM_NAME MATCHES Windows)
# platform check
# source: https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-To-Write-Platform-Checks
    set(SDL2_DIR "lib/windows/SDL2-devel-2.0.10-VC/SDL2-2.0.10")
else()
    set(SDL2_DIR "/usr/lib/i386-linux-gnu/cmake/SDL2")
endif()
find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)

add_library(cssHack SHARED
        ${HACK_SOURCES}
        ${IMGUI_SOURCES}
        ${BOYERMOOREDONTCARE_SOURCES}
        )

#set(OpenGL_GL_PREFERENCE LEGACY)
#find_package(OpenGL REQUIRED)
target_include_directories(cssHack
        PRIVATE ${CMAKE_SOURCE_DIR}/src
        PRIVATE ${IMGUI_INCLUDE_DIRS}
        PRIVATE ${BOYERMOOREDONTCARE_INCLUDE_DIRS}
        PRIVATE ${GLM_INCLUDE_DIRS}
        PRIVATE ${SDL2_INCLUDE_DIRS}
)
        
if(CMAKE_SYSTEM_NAME MATCHES Windows)
    find_package(OpenGL REQUIRED)
    target_link_libraries(cssHack
            PRIVATE ${CMAKE_THREAD_LIBS_INIT}
            PRIVATE ${SDL2_LIBRARIES}
            PRIVATE ${OPENGL_LIBRARIES}
    )
    target_include_directories(cssHack
        PRIVATE ${OPENGL_INCLUDE_DIRS}
    )
else()
    target_link_libraries(cssHack
            PRIVATE ${CMAKE_THREAD_LIBS_INIT}
            PRIVATE ${SDL2_LIBRARIES}
            #PRIVATE "/usr/lib/i386-linux-gnu/libGL.so.1"
            )
endif()

###########################
# compile / link flags
###########################
if(CMAKE_SYSTEM_NAME MATCHES Windows)
else()
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
endif()
