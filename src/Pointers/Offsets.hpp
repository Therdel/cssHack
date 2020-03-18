//
// Created by therdel on 16.06.19.
//
#pragma once

#include <array>

namespace Offsets {
	// Aimbot
#ifdef __linux__
    constexpr ptrdiff_t client_viewAngleVis = 0xBC2294;

    constexpr ptrdiff_t localplayer_off_punch = 0xE28;
    constexpr ptrdiff_t localplayer_off_team = 0x8c;
    constexpr ptrdiff_t playerArray_off = 0x28;
    constexpr ptrdiff_t localplayer_off_targetId = 0x14D0;

#else
	constexpr ptrdiff_t client_viewAngleVis = 0x504644;
	constexpr ptrdiff_t localplayer_off_punch= 0xE48;
	constexpr ptrdiff_t localplayer_off_team = 0x9c;
	constexpr ptrdiff_t playerArray_off = 0x28;
	constexpr ptrdiff_t localplayer_off_targetId = 0x14f0;
#endif

	// Hack
#ifdef __linux__
	constexpr ptrdiff_t materialsystem_isIngame = 0x15B869;
	constexpr ptrdiff_t vgui2_isInMenu = 0x68720;
#else
	constexpr ptrdiff_t materialsystem_isIngame = 0x1215C9;
#endif
	// Input
#ifdef __linux__
//	constexpr ptrdiff_t launcher_sdl_pollEvent_caller = 0x32F5B;
	// 2019.10.11
	constexpr ptrdiff_t launcher_sdl_pollEvent_caller = 0x32F7B;
#else
	constexpr ptrdiff_t inputsystem_sdl_pollEvent_caller = 0x3A22;
#endif
	// ESP
#ifdef __linux__
	constexpr ptrdiff_t launcher_sdl_swapWindow_caller = 0x30EBC;
	constexpr ptrdiff_t client_matViewModel = 0xBF2BE0;
	constexpr ptrdiff_t engine_screenDimensions = 0xD23A54;
	constexpr ptrdiff_t engine_fov_horizontal = 0xC9EAF8;
#else
#endif
	// GamePointerUpdater
#ifdef __linux
	constexpr ptrdiff_t client_op_localplayer_update = 0x56BAAA;
	constexpr ptrdiff_t client_op_localplayer_invalidate = 0x573390;
#else
#endif
	// Wallhack
#ifdef __linux__
	constexpr ptrdiff_t shaderapidx9_drawIndexedPrimitive_caller = 0x2DABC;
#else
#endif
}
