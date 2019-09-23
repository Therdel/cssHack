//
// Created by therdel on 04.09.19.
//
#pragma once

#include "ForeignPointerCRTP.hpp"
#include "../../Vec3f.hpp"

namespace PointersCRTP {
	auto localplayer() -> ForeignPtrOffset<ForeignLibraryPtrBase> &;

	auto
	player_punch() -> ForeignPtrOffset<ForeignPtrOffset<ForeignLibraryPtrBase>, Vec3f> &;

	auto doAttack() -> ForeignPtrOffset<ForeignLibraryPtrBase, int> &;

	auto op_on_update_viewAngle() -> ForeignPtrOffset<ForeignLibraryPtrBase> &;

	auto op_on_update_viewAngleVis() -> ForeignPtrOffset<ForeignLibraryPtrBase> &;
}
