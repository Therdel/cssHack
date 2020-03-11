//
// Created by therdel on 05.09.19.
//

#include "GamePointerDef.hpp"
#include "Offsets.hpp"
#include <array>

using namespace libNames;
using namespace Offsets;

namespace GamePointerDef {
template<typename T>
Base<T>::Base(std::string_view libName,
			  std::vector<ptrdiff_t> offsets,
			  OffsetType lastOffsetType)
: libName(libName)
, offsets(std::move(offsets))
, lastOffsetType(lastOffsetType) {}

template<typename T, typename BaseT>
Composite<T, BaseT>::Composite(Base<BaseT> const& base,
							   std::vector<ptrdiff_t> offsets,
							   OffsetType lastOffsetType)
: base(base)
, offsets(std::move(offsets))
, lastOffsetType(lastOffsetType) {}

// TODO: Find out why we need the explicit namespace here for linking
// input
auto op_sdl_pollEvent_call() -> const Base<>& {
	static auto def =
#ifdef __linux__
		Base{launcher, {launcher_sdl_pollEvent_caller}};
#else
		Base{inputsystem, {inputsystem_sdl_pollEvent_caller}};
#endif
	return def;
};

// bunnyhop
auto onGround() -> const Base<int>& {
	static Base<int> def{client, {client_onGround}};
	return def;
}
auto doJump() -> const Base<int>& {
	static Base<int> def{client, {client_doJump}};
	return def;
}
auto op_onGround_inc() -> const Base<>& {
	static Base<> def{client, {client_onGround_op_inc}};
	return def;
}
auto op_onGround_dec() -> const Base<>& {
	static Base<> def{client, {client_onGround_op_dec}};
	return def;
}

// aimbot
auto localplayer() -> const Base<>& {
	static Base<> def{client, {client_localplayer}, OffsetType::DEREFERENCE};
	return def;
}
auto playerPos() -> const Base<Vec3f>& {
	static Base<Vec3f> def{engine, {engine_player_pos}};
	return def;
}
auto aimAngles() -> const Base<Vec3f>& {
	static Base<Vec3f> def{engine, {engine_viewAngles}};
	return def;
}
auto visualAngles() -> const Base<Vec3f>& {
	static Base<Vec3f> def{client, {client_viewAngleVis}};
	return def;
}
auto punchAngles() -> const Composite<Vec3f>& {
	static Composite<Vec3f> def{localplayer(), {client_punch_p_off}};
	return def;
}
auto playerTeam() -> const Composite<Player::TEAM>& {
	static Composite<Player::TEAM> def{localplayer(), {client_player_team_p_off}};
	return def;
}
auto players() -> const Base<std::array<Player, 64>>& {
	static Base<std::array<Player, 64>> def{client, {client_player_p_base, client_player_p_off}};
	return def;
}
auto targetId() -> const Composite<int>& {
	static Composite<int> def{localplayer(), {client_target_id_p_off}};
	return def;
}
auto doAttack() -> const Base<int>& {
	static Base<int> def{client, {client_doAttack}};
	return def;
}
auto op_viewAngles_update() -> const Base<>& {
	static Base<> def{engine, {engine_op_viewAngle_update}};
	return def;

}
auto op_viewAnglesVis_update() -> const Base<>& {
	static Base<> def{client, {client_op_viewAngleVis_update}};
	return def;
}

// hack
auto isIngame() -> const Base<uint8_t>& {
	static Base<uint8_t> def{materialsystem, {materialsystem_isIngame}};
	return def;
}
auto isInMenu() -> const Base<uint32_t>& {
	static Base<uint32_t> def{vgui2, {vgui2_isInMenu}};
	return def;
}

// GamePointerUpdater
auto op_localplayer_update() -> const Base<>& {
	static Base<> def{client, {client_op_localplayer_update}};
	return def;
}
auto op_localplayer_invalidate() -> const Base<>& {
	static Base<> def{client, {client_op_localplayer_invalidate}};
	return def;
}
}
