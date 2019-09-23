//
// Created by therdel on 05.09.19.
//

#include "GamePointerDef.hpp"
#include "Offsets.hpp"

using namespace GamePointerDef;
using namespace libNames;
using namespace Offsets;

// TODO: Find out why we need the explicit namespace here for linking
// bunnyhop
Base<int>
		GamePointerDef::onGround
		{client, {client_onGround}};
Base<int>
		GamePointerDef::doJump
		{client, {client_doJump}};
Base<>
		GamePointerDef::op_onGround_inc
		{client, {client_onGround_op_inc}};
Base<>
		GamePointerDef::op_onGround_dec
		{client, {client_onGround_op_dec}};
// aimbot
Base<>
		GamePointerDef::localplayer
		{client, {client_localplayer}, OffsetType::DEREFERENCE};
Base<Vec3f>
		GamePointerDef::playerPos
		{engine, {engine_player_pos}};
Base<Vec3f>
		GamePointerDef::aimAngles
		{engine, {engine_viewAngles}};
Base<Vec3f>
		GamePointerDef::visualAngles
		{client, {client_viewAngleVis}};
Composite<Vec3f>
		GamePointerDef::punchAngles
		{localplayer, {client_punch_p_off}};
Composite<Player::TEAM>
		GamePointerDef::playerTeam
		{localplayer, {client_player_team_p_off}};
Base<std::array<Player, 64>>
		GamePointerDef::players
		{client, {client_player_p_base, client_player_p_off}};
Composite<int>
		GamePointerDef::targetId
		{localplayer, {client_target_id_p_off}};
Base<int>
		GamePointerDef::doAttack
		{client, {client_doAttack}};
Base<>
		GamePointerDef::op_viewAngles_update
		{engine, {engine_op_viewAngle_update}};
Base<>
		GamePointerDef::op_viewAnglesVis_update
		{client, {client_op_viewAngleVis_update}};
// hack
Base<uint8_t>
		GamePointerDef::isIngame
		{materialsystem, {materialsystem_isIngame}};
Base<uint32_t >
		GamePointerDef::isInMenu
		{vgui2, {vgui2_isInMenu}};