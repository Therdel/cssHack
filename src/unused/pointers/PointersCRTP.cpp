//
// Created by therdel on 04.09.19.
//
#include "PointersCRTP.hpp"
#include "../../Pointers/Offsets.hpp"
#include "ForeignPointerBuilderCRTP.hpp"

auto PointersCRTP::localplayer()
-> ForeignPtrOffset<ForeignLibraryPtrBase> & {
	static auto p_localplayer =
			ForeignPtrBuilder::base(libNames::client)
					.add_offset(Offsets::client_localplayer, OffsetType::DEREFERENCE)
					.build();
//	static ForeignLibraryPtrBase p_client_base(libNames::client);
//	static ForeignPtrOffset p_localplayer(p_client_base, Offsets::client_localplayer, OffsetType::DEREFERENCE);

	return p_localplayer;
}

auto PointersCRTP::player_punch()
-> ForeignPtrOffset<ForeignPtrOffset<ForeignLibraryPtrBase>, Vec3f> & {
//	static auto p_player_punch = ForeignPtrBuilder::base(libNames::client);
	using localplayer_t = std::remove_reference_t<decltype(localplayer())>;
	static ForeignPtrOffset<localplayer_t, Vec3f> p_player_punch(localplayer(), Offsets::client_punch_p_off,
	                                                             OffsetType::PLAIN_OFFSET);

	return p_player_punch;
}

auto PointersCRTP::doAttack()
-> ForeignPtrOffset<ForeignLibraryPtrBase, int> & {
	static ForeignLibraryPtrBase p_client_base(libNames::client);
	static ForeignPtrOffset<decltype(p_client_base), int> p_doAttack(p_client_base, Offsets::client_doAttack,
	                                                                 OffsetType::PLAIN_OFFSET);

	return p_doAttack;
}

auto PointersCRTP::op_on_update_viewAngle() -> ForeignPtrOffset<ForeignLibraryPtrBase> & {
	static ForeignLibraryPtrBase p_engine_base(libNames::engine);
	static ForeignPtrOffset p_op_on_update_viewAngle_raw(p_engine_base, Offsets::engine_op_viewAngle_update,
	                                                     OffsetType::PLAIN_OFFSET);

	return p_op_on_update_viewAngle_raw;
}

auto PointersCRTP::op_on_update_viewAngleVis() -> ForeignPtrOffset<ForeignLibraryPtrBase> & {
	static ForeignLibraryPtrBase p_client_base(libNames::client);
	static ForeignPtrOffset p_op_on_update_viewAngleVis_raw(p_client_base, Offsets::client_op_viewAngleVis_update,
	                                                        OffsetType::PLAIN_OFFSET);

	return p_op_on_update_viewAngleVis_raw;
}