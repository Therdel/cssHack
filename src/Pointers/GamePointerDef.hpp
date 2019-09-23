//
// Created by therdel on 05.09.19.
//
#pragma once

#include <vector>

#include "libNames.hpp"
#include "GamePointer.hpp"
#include "../Vec3f.hpp"
#include "../Player.hpp"

namespace GamePointerDef {
	template<typename T=uintptr_t>
	struct Base {
		Base(LibName const &libName,
		     std::vector<ptrdiff_t> offsets,
		     OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET)
				: libName(libName)
				, offsets(std::move(offsets))
				, lastOffsetType(lastOffsetType) {}

		LibName const &libName;
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

	// bunnyhop
	extern Base<int> onGround;
	extern Base<int> doJump;
	extern Base<> op_onGround_inc;
	extern Base<> op_onGround_dec;
	// aimbot
	extern Base<> localplayer;
	extern Base<Vec3f> playerPos;
	extern Base<Vec3f> aimAngles;
	extern Base<Vec3f> visualAngles;
	extern Composite<Vec3f> punchAngles;
	extern Composite<Player::TEAM> playerTeam;
	extern Base<std::array<Player, 64>> players;
	extern Composite<int> targetId;
	extern Base<int> doAttack;
	extern Base<> op_viewAngles_update;
	extern Base<> op_viewAnglesVis_update;
	// hack
	extern Base<uint8_t> isIngame;
	extern Base<uint32_t> isInMenu;
}
