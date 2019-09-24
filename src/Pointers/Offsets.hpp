//
// Created by therdel on 16.06.19.
//
#pragma once

#include <array>

#include "GamePointer.hpp"

namespace Offsets {
	// Bunnyhop
#ifdef __linux__
	constexpr ptrdiff_t client_onGround = 0xB9C5B0;
	constexpr ptrdiff_t client_doJump = 0xBEC388;
	constexpr ptrdiff_t client_onGround_op_inc = 0x40B76A;
	constexpr ptrdiff_t client_onGround_op_dec = 0x40B816;
#else
	constexpr ptrdiff_t client_onGround = 0x4F60C4;
	constexpr ptrdiff_t client_doJump = 0x4f3b3c;
	constexpr ptrdiff_t client_onGround_op_inc = 0x168C6E;
	constexpr ptrdiff_t client_onGround_op_dec = 0x16A7C0;
#endif

	// Aimbot
#ifdef __linux__
//	constexpr ptrdiff_t client_localplayer = 0xBA3E74;
	constexpr ptrdiff_t client_localplayer = 0xBCE5F0;
	constexpr ptrdiff_t engine_viewAngles = 0xB37058;
	constexpr ptrdiff_t client_viewAngleVis = 0xBC1CB4;
	constexpr ptrdiff_t client_punch_p_base = client_localplayer;
	constexpr ptrdiff_t client_punch_p_off = 0xE28;

	constexpr ptrdiff_t engine_player_pos = 0xB74F30;
	constexpr ptrdiff_t client_player_team_p_base = client_localplayer;
	constexpr ptrdiff_t client_player_team_p_off = 0x8c;
	constexpr ptrdiff_t client_player_p_base = 0xBE7220;
	constexpr ptrdiff_t client_player_p_off = 0x28;
	constexpr ptrdiff_t client_target_id_p_base = client_localplayer;
	constexpr ptrdiff_t client_target_id_p_off = 0x14D0;
	constexpr ptrdiff_t client_doAttack = 0xBEC418;

	constexpr ptrdiff_t engine_op_viewAngle_update = 0x38AFE6;
	constexpr ptrdiff_t client_op_viewAngleVis_update = 0x3B0E1A;

#else
	constexpr ptrdiff_t materialsystem_isIngame = 0x1215C9;
		constexpr ptrdiff_t client_localplayer = 0x4C6708;
		constexpr ptrdiff_t engine_viewAngles = 0x47F1B4;
		constexpr ptrdiff_t client_viewAngleVis = 0x504644;
		constexpr ptrdiff_t client_punch_p_base = client_localplayer;
		constexpr ptrdiff_t client_punch_p_off = 0xE48;
		constexpr ptrdiff_t engine_player_pos = 0x3D64BC;
		constexpr ptrdiff_t client_player_team_p_base = client_localplayer;
		constexpr ptrdiff_t client_player_team_p_off = 0x9c;
		constexpr ptrdiff_t client_player_p_base = 0x50C150;
		constexpr ptrdiff_t client_player_p_off = 0x28;
		constexpr ptrdiff_t client_target_id_p_base = client_localplayer;
		constexpr ptrdiff_t client_target_id_p_off = 0x14f0;
		constexpr ptrdiff_t client_doAttack = 0x4F3B48;

		constexpr ptrdiff_t engine_op_viewAngle_update = 0xA5447;
		constexpr ptrdiff_t client_op_viewAngleVis_update = 0x68E5F;
#endif

	// Hack
#ifdef __linux__
	constexpr ptrdiff_t materialsystem_isIngame = 0x15B869;
	constexpr ptrdiff_t vgui2_isInMenu = 0x68720;
#else
	constexpr ptrdiff_t materialsystem_isIngame = 0x1215C9;
#endif
	// Input
	constexpr ptrdiff_t launcher_sdl_pollEvent_caller = 0x32F5B;
	// ESP
	constexpr ptrdiff_t launcher_sdl_swapWindow_caller = 0x30EBC;
	constexpr ptrdiff_t client_matViewModel = 0xBF2BE0;
	constexpr ptrdiff_t engine_screenDimensions = 0xD23A54;
	constexpr ptrdiff_t engine_fov_horizontal = 0xC9EAF8;
	// GamePointerUpdater
	constexpr ptrdiff_t client_op_localplayer_update = 0x56BAAA;
	constexpr ptrdiff_t client_op_localplayer_invalidate = 0x573390;
	// Wallhack
	constexpr ptrdiff_t shaderapidx9_drawIndexedPrimitive_caller = 0x2DABC;
}
