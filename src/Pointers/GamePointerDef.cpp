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

auto extractAddressFromCode(const SignatureAOI& signature) -> uintptr_t {
	uintptr_t* pointer_to_address_in_code = reinterpret_cast<uintptr_t*>(MemoryScanner::scanSignatureExpectOneResult(signature));
	return *pointer_to_address_in_code;
}

// bunnyhop
auto onGround() -> const RawPointer<int>& {
	static RawPointer<int> rawPointer = { extractAddressFromCode(Signatures::onGround) };
	return rawPointer;
}
auto doJump() -> const RawPointer<int>& {
	static RawPointer<int> rawPointer = { extractAddressFromCode(Signatures::doJump) };
	return rawPointer;
}

// aimbot
auto localplayer() -> const RawPointer<>& {
	// the signature contains an address to our wanted pointer to the localplayer base.
	static auto* pointerToPointer = reinterpret_cast<uintptr_t*>(extractAddressFromCode(Signatures::localplayer_base));
	static RawPointer<> rawPointer = { *pointerToPointer };
	return rawPointer;
}
auto playerPos() -> const RawPointer<Vec3f>& {
	static RawPointer<Vec3f> rawPointer = { extractAddressFromCode(Signatures::playerPos) };
	return rawPointer;
}
auto aimAngles() -> const RawPointer<Vec3f>& {
	static RawPointer<Vec3f> rawPointer = { extractAddressFromCode(Signatures::aimAngles) };
	return rawPointer;
}
auto aimAnglesVisual() -> const Base<Vec3f>& {
	static Base<Vec3f> def{client, {client_viewAngleVis}};
	return def;
}
auto punchAngles() -> const Composite<Vec3f, RawPointer<>>& {
	static Composite<Vec3f, RawPointer<>> def{localplayer(), {localplayer_off_punch}};
	return def;
}
auto playerTeam() -> const Composite<Player::TEAM, RawPointer<>>& {
	static Composite<Player::TEAM, RawPointer<>> def{localplayer(), {localplayer_off_team}};
	return def;
}
auto players() -> const Composite<std::array<Player, 64>, RawPointer<>>& {
	// the signature contains an address to our wanted pointer to the playerArray base.
	static auto* pointerToPointer = reinterpret_cast<uintptr_t*>(extractAddressFromCode(Signatures::playerArray_base));
	static RawPointer<> rawPointer = { *pointerToPointer };
	static Composite<std::array<Player, 64>, RawPointer<>> def{ rawPointer, {playerArray_off} };
	return def;
}
auto targetId() -> const Composite<int, RawPointer<>>& {
	static Composite<int, RawPointer<>> def{localplayer(), {localplayer_off_targetId}};
	return def;
}
auto doAttack() -> const RawPointer<int>& {
	static RawPointer<int> rawPointer = { extractAddressFromCode(Signatures::doAttack) };
	return rawPointer;
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
