//
// Created by therdel on 05.09.19.
//
#pragma once

#include <vector>
#include <array>

#include "GamePointer.hpp"
#include "../Vec3f.hpp"
#include "../Player.hpp"

namespace GamePointerDef {
	template<typename T = uintptr_t>
	struct RawPointer {
		RawPointer(uintptr_t address);
		uintptr_t _address;
	};

	template<typename T = uintptr_t>
	struct RawOffset {
		RawOffset(ptrdiff_t offset);
		ptrdiff_t _offset;
	};

	template<typename T=uintptr_t>
	struct Base {
		Base(std::string_view libName,
			 std::vector<ptrdiff_t> offsets,
			 OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET);

		std::string_view libName;
		std::vector<ptrdiff_t> offsets;
		OffsetType lastOffsetType;
	};

	template<typename T=uintptr_t, typename BaseDef = Base<uintptr_t>>
	struct Composite {
		Composite(BaseDef const& base,
				  std::vector<ptrdiff_t> offsets,
				  OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET);

		BaseDef const &base;
		std::vector<ptrdiff_t> offsets;
		OffsetType lastOffsetType;
	};

	// input
	auto op_sdl_pollEvent_call() -> const Base<>&;
	// bunnyhop
	auto onGround() -> const RawPointer<int>&;
	auto doJump() -> const RawPointer<int>&;
	// aimbot
	auto localplayer() -> const RawPointer<>&;
	auto playerPos() -> const RawPointer<Vec3f>&;
	auto aimAngles() -> const RawPointer<Vec3f>&;
	auto aimAnglesVisual() -> const Base<Vec3f>&;
	auto punchAngles() -> const Composite<Vec3f, RawPointer<>>&;
	auto playerTeam() -> const Composite<Player::TEAM, RawPointer<>>&;
	auto players() -> const Composite<std::array<Player, 64>, RawPointer<>>&;
	auto targetId() -> const Composite<int, RawPointer<>>&;
	auto doAttack() -> const RawPointer<int>&;
	// hack
	auto isIngame() -> const Base<uint8_t>&;
	auto isInMenu() -> const Base<uint32_t> &;
	// GamePointerUpdater
	auto op_localplayer_update() -> const Base<> &;
	auto op_localplayer_invalidate() -> const Base<> &;
}
