//
// Created by therdel on 05.09.19.
//
#pragma once

#include "GamePointer.hpp"
#include "../Vec3f.hpp"
#include "../Player.hpp"

namespace Pointers {
	std::shared_ptr<GamePointer<>> const &localplayer();

	std::shared_ptr<GamePointer<Vec3f>> const &player_pos();

	std::shared_ptr<GamePointer<Vec3f>> const &player_angles();

	std::shared_ptr<GamePointer<Vec3f>> const &player_angles_vis();

	std::shared_ptr<GamePointer<Vec3f>> const &player_punch();

	std::shared_ptr<GamePointer<Player::TEAM>> const &player_team();

	std::shared_ptr<GamePointer<std::array<Player, 64>>> &players();

	std::shared_ptr<GamePointer<int>> &crosshair_target_id();

	std::shared_ptr<GamePointer<int>> const &doAttack();

	std::shared_ptr<GamePointer<>> const &op_on_viewAngle_update();

	std::shared_ptr<GamePointer<>> const &op_on_viewAngleVis_update();
}
