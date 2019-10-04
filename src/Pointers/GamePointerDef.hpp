//
// Created by therdel on 05.09.19.
//
#pragma once

#include <vector>

#include "GamePointer.hpp"
#include "../Vec3f.hpp"
#include "../Player.hpp"

namespace GamePointerDef {
	template<typename T=uintptr_t>
	struct Base {
		Base(std::string_view libName,
		     std::vector<ptrdiff_t> offsets,
		     OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET)
				: libName(libName)
				, offsets(std::move(offsets))
				, lastOffsetType(lastOffsetType) {}

		std::string_view const &libName;
		std::vector<ptrdiff_t> offsets;
		OffsetType lastOffsetType;
	};

	template<typename T=uintptr_t, typename BaseT=uintptr_t>
	struct Composite {
		Composite(Base<BaseT> const &base,
		          std::vector<ptrdiff_t> offsets,
		          OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET)
				: base(base)
				, offsets(std::move(offsets))
				, lastOffsetType(lastOffsetType) {}

		Base<BaseT> const &base;
		std::vector<ptrdiff_t> offsets;
		OffsetType lastOffsetType;
	};

	// input
	const Base<> &op_sdl_pollEvent_call();
	// bunnyhop
	const Base<int> &onGround();
	const Base<int> &doJump();
	const Base<> &op_onGround_inc();
	const Base<> &op_onGround_dec();
	// aimbot
	const Base<> &localplayer();
	const Base<Vec3f> &playerPos();
	const Base<Vec3f> &aimAngles();
	const Base<Vec3f> &visualAngles();
	const Composite<Vec3f> &punchAngles();
	const Composite<Player::TEAM> &playerTeam();
	const Base<std::array<Player, 64>> &players();
	const Composite<int> &targetId();
	const Base<int> &doAttack();
	const Base<> &op_viewAngles_update();
	const Base<> &op_viewAnglesVis_update();
	// hack
	const Base<uint8_t> &isIngame();
	const Base<uint32_t> &isInMenu();
	// GamePointerUpdater
	const Base<> &op_localplayer_update();
	const Base<> &op_localplayer_invalidate();
}
