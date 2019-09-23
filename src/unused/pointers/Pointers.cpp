//
// Created by therdel on 05.09.19.
//s
#include "Pointers.hpp"
#include "GamePointerFactory.hpp"
#include "Offsets.hpp"

using namespace Offsets;
using namespace libNames;

namespace Pointers {
	std::shared_ptr<GamePointer<>> const &localplayer() {
		static auto p_localPlayer = GamePointerFactory::create(client,
		                                                       {client_localplayer},
		                                                       OffsetType::DEREFERENCE);
		return p_localPlayer;
	}

	std::shared_ptr<GamePointer<Vec3f>> const &player_pos() {
		static auto p = GamePointerFactory::create<Vec3f>(engine, {engine_player_pos});
		return p;
	}

	std::shared_ptr<GamePointer<Vec3f>> const &player_angles() {
		static auto p = GamePointerFactory::create<Vec3f>(engine, {engine_viewAngles});
		return p;
	}

	std::shared_ptr<GamePointer<Vec3f>> const &player_angles_vis() {
		static auto p = GamePointerFactory::create<Vec3f>(client, {client_viewAngleVis});
		return p;
	}

	std::shared_ptr<GamePointer<Vec3f>> const &player_punch() {
		static auto p = GamePointerFactory::create<Vec3f>(localplayer(), {client_punch_p_off});
		return p;
	}

	std::shared_ptr<GamePointer<Player::TEAM>> const &player_team() {
		static auto p = GamePointerFactory::create<Player::TEAM>(localplayer(), {client_player_team_p_off});
		return p;
	}

	std::shared_ptr<GamePointer<std::array<Player, 64>>> &players() {
		static auto p = GamePointerFactory::create<std::array<Player, 64>>(client,
		                                                                   {client_player_p_base,
		                                                                client_player_p_off});
		return p;
	}

	std::shared_ptr<GamePointer<int>> &crosshair_target_id() {
		static auto p = GamePointerFactory::create<int>(localplayer(), {client_target_id_p_off});
		return p;
	}

	std::shared_ptr<GamePointer<int>> const &doAttack() {
		static auto p = GamePointerFactory::create<int>(client, {client_doAttack});
		return p;
	}

	std::shared_ptr<GamePointer<>> const &op_on_viewAngle_update() {
		static auto p_op_on_update_viewAngle = GamePointerFactory::create(engine, {engine_op_viewAngle_update});
		return p_op_on_update_viewAngle;
	}

	std::shared_ptr<GamePointer<>> const &op_on_viewAngleVis_update() {
		static auto p_op_on_update_viewAngleVis = GamePointerFactory::create(client, {client_op_viewAngleVis_update});
		return p_op_on_update_viewAngleVis;
	}
}
