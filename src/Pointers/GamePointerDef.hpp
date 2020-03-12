//
// Created by therdel on 05.09.19.
//
#pragma once

#include <vector>

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

	template<typename T=uintptr_t, typename BaseT=uintptr_t>
	struct Composite {
		Composite(Base<BaseT> const& base,
				  std::vector<ptrdiff_t> offsets,
				  OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET);

		Base<BaseT> const &base;
		std::vector<ptrdiff_t> offsets;
		OffsetType lastOffsetType;
	};

	// input
	auto op_sdl_pollEvent_call() -> const Base<>&;
	// bunnyhop
	auto onGround() -> const RawPointer<int>&;
	auto doJump() -> const Base<int>&;
	// aimbot
	auto localplayer() -> const Base<>&;
	auto playerPos() -> const Base<Vec3f>&;
	auto aimAngles() -> const Base<Vec3f>&;
	auto visualAngles() -> const Base<Vec3f>&;
	auto punchAngles() -> const Composite<Vec3f>&;
	auto playerTeam() -> const Composite<Player::TEAM>&;
	auto players() -> const Base<std::array<Player, 64>>&;
	auto targetId() -> const Composite<int>&;
	auto doAttack() -> const Base<int>&;
	auto op_viewAngles_update() -> const Base<>&;
	auto op_viewAnglesVis_update() -> const Base<>&;
	// hack
	auto isIngame() -> const Base<uint8_t>&;
	auto isInMenu() -> const Base<uint32_t> &;
	// GamePointerUpdater
	auto op_localplayer_update() -> const Base<> &;
	auto op_localplayer_invalidate() -> const Base<> &;
}
