//
// Created by therdel on 05.09.19.
//
#include <array>

#include "GamePointerDef.hpp"
#include "Offsets.hpp"
#include "MemoryScanner/MemoryScanner.hpp"
#include "Signatures.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX
#include "Log.hpp"

using namespace libNames;
using namespace Offsets;

namespace GamePointerDef {
template<typename T>
RawPointer<T>::RawPointer(uintptr_t address)
: _address(address) {}

template<typename T>
RawOffset<T>::RawOffset(ptrdiff_t offset)
: _offset(offset) {}

template<typename T>
Base<T>::Base(std::string_view libName,
			  std::vector<ptrdiff_t> offsets,
			  OffsetType lastOffsetType)
: libName(libName)
, offsets(std::move(offsets))
, lastOffsetType(lastOffsetType) {}

template<typename T, typename BaseDef>
Composite<T, BaseDef>::Composite(BaseDef const& base,
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
auto onGround() -> const RawPointer<int>& {
	static int** pointer_to_address_in_code = reinterpret_cast<int**>(MemoryScanner::scanSignatureExpectOneResult(Signatures::onGround));
	static RawPointer<int> address = { reinterpret_cast<uintptr_t>(*pointer_to_address_in_code) };
	return address;
}
auto doJump() -> const RawPointer<int>& {
	static int** pointer_to_address_in_code = reinterpret_cast<int**>(MemoryScanner::scanSignatureExpectOneResult(Signatures::doJump));
	static RawPointer<int> address = { reinterpret_cast<uintptr_t>(*pointer_to_address_in_code) };
	return address;
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
