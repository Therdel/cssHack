//
// Created by therdel on 16.06.19.
//
#pragma once

#include <array>

enum class OffsetType {
	DEREFERENCE,
	PLAIN_OFFSET
};

namespace Offsets {
	// Aimbot
#ifdef __linux__
    constexpr ptrdiff_t client_viewAngleVis = 0xBC3E14;

    constexpr ptrdiff_t localplayer_off_team = 0x8C;
    constexpr ptrdiff_t localplayer_off_punch = 0xE28;
    constexpr ptrdiff_t localplayer_off_targetId = 0x14D0;
    constexpr ptrdiff_t playerArray_off = 0x28;
    constexpr ptrdiff_t client_radarstruct_playerArray_base = 0xBE9380;
    constexpr ptrdiff_t radarstruct_playerArray_off = 0x28;

#else
	constexpr ptrdiff_t client_viewAngleVis = 0x504644;
	constexpr ptrdiff_t localplayer_off_team = 0x9c;
	constexpr ptrdiff_t localplayer_off_punch= 0xE48;
	constexpr ptrdiff_t localplayer_off_targetId = 0x14f0;
	constexpr ptrdiff_t playerArray_off = 0x28;
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
	constexpr ptrdiff_t launcher_sdl_pollEvent_caller = 0x3325B;
#else
	constexpr ptrdiff_t inputsystem_sdl_pollEvent_caller = 0x3A22;
#endif
	// ESP
#ifdef __linux__
	constexpr ptrdiff_t launcher_sdl_swapWindow_caller = 0x311BC;
	constexpr ptrdiff_t engine_screenDimensions = 0xD20014;
	constexpr ptrdiff_t engine_fov_vertical_degrees = 0xC720A4;	// TIP: game cvar "fov" doesn't show real FOV. With fov=90, the real fov_y is ~73°
#else
#endif
	// Wallhack
#ifdef __linux__
	constexpr ptrdiff_t shaderapidx9_drawIndexedPrimitive_caller = 0x2DABC;
#else
#endif
}
