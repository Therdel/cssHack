//
// Created by therdel on 05.09.19.
//

#include "GamePointerDef.hpp"
#include "Offsets.hpp"
#include <array>

using namespace GamePointerDef;
using namespace libNames;
using namespace Offsets;

// TODO: Find out why we need the explicit namespace here for linking
// input
const Base<> &
GamePointerDef::op_sdl_pollEvent_call() {
	static auto def =
#ifdef __linux__
		Base{launcher,{launcher_sdl_pollEvent_caller}};
#else
		Base{inputsystem,   {inputsystem_sdl_pollEvent_caller}};
#endif
	return def;
};
// bunnyhop
const Base<int> &GamePointerDef::onGround() {
	static Base<int> def{client, {client_onGround}};
	return def;
}

const Base<int> &GamePointerDef::doJump() {
	static Base<int> def{client, {client_doJump}};
	return def;
}

const Base<> &
		GamePointerDef::op_onGround_inc() {
	static Base<> def{client, {client_onGround_op_inc}};
	return def;
}
const Base<> &
		GamePointerDef::op_onGround_dec() {
	static Base<> def{client, {client_onGround_op_dec}};
	return def;
}

// aimbot
const Base<> &
		GamePointerDef::localplayer() {
	static Base<> def{client, {client_localplayer}, OffsetType::DEREFERENCE};
	return def;
}
const Base<Vec3f> &
		GamePointerDef::playerPos() {
	static Base<Vec3f> def{engine, {engine_player_pos}};
	return def;
}
const Base<Vec3f> &
		GamePointerDef::aimAngles() {
	static Base<Vec3f> def{engine, {engine_viewAngles}};
	return def;
}
const Base<Vec3f> &
		GamePointerDef::visualAngles() {
	static Base<Vec3f> def{client, {client_viewAngleVis}};
	return def;
}
const Composite<Vec3f> &
		GamePointerDef::punchAngles() {
	static Composite<Vec3f> def{localplayer(), {client_punch_p_off}};
	return def;
}
const Composite<Player::TEAM> &
		GamePointerDef::playerTeam() {
	static Composite<Player::TEAM> def{localplayer(), {client_player_team_p_off}};
	return def;
}
const Base<std::array<Player, 64>> &
		GamePointerDef::players() {
	static Base<std::array<Player, 64>> def{client, {client_player_p_base, client_player_p_off}};
	return def;
}
const Composite<int> &
		GamePointerDef::targetId() {
	static Composite<int> def{localplayer(), {client_target_id_p_off}};
	return def;
}
const Base<int> &
		GamePointerDef::doAttack() {
	static Base<int> def{client, {client_doAttack}};
	return def;
}
const Base<> &
		GamePointerDef::op_viewAngles_update() {
	static Base<> def{engine, {engine_op_viewAngle_update}};
	return def;

}
const Base<> &
		GamePointerDef::op_viewAnglesVis_update() {
	static Base<> def{client, {client_op_viewAngleVis_update}};
	return def;
}
// hack
const Base<uint8_t> &
		GamePointerDef::isIngame() {
	static Base<uint8_t> def{materialsystem, {materialsystem_isIngame}};
	return def;
}
const Base<uint32_t> &
		GamePointerDef::isInMenu() {
	static Base<uint32_t> def{vgui2, {vgui2_isInMenu}};
	return def;
}
// GamePointerUpdater
const Base<> &
		GamePointerDef::op_localplayer_update() {
	static Base<> def{client, {client_op_localplayer_update}};
	return def;
}
const Base<> &
		GamePointerDef::op_localplayer_invalidate() {
	static Base<> def{client, {client_op_localplayer_invalidate}};
	return def;
}