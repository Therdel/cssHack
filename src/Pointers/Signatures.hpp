//
// Created by therdel on 12.10.19.
//
#pragma once
#include <string_view>

#include "BoyerMooreDontCare/Signature.hpp"

/// Byte pattern used to find a memory location.
/// Signatures are defined by a C-string describing the bytes that make up the wanted piece of memory.
/// Known bytes are specified in hex, variable/"don't care" bytes using '??'
///
/// example: "74 4E E8 ?? ?? ?? ??"
/// would match every byte array of length 7 with specified first 3 bytes.
struct SignatureAOI {
	SignatureAOI(std::string_view libName, Signature signature, ptrdiff_t aoi_offset, size_t aoi_length)
	: libName(libName)
	, signature(std::move(signature))
	, aoi_offset(aoi_offset)
	, aoi_length(aoi_length) {}

	std::string_view libName;
	Signature signature;
	ptrdiff_t aoi_offset;
	size_t aoi_length;
};

namespace Signatures {
	// bunnyhop
	extern SignatureAOI onGround_op_land;
	extern SignatureAOI onGround_op_leave;
	extern SignatureAOI onGround;
	extern SignatureAOI doJump;

	// aimbot
	extern SignatureAOI aimAngles_x_op_read;
	extern SignatureAOI aimAngles;
	extern SignatureAOI aimAnglesVisual_update;
	extern SignatureAOI localplayer_base;
	extern SignatureAOI playerArray_base;
	extern SignatureAOI playerPos;
	extern SignatureAOI doAttack_op_read;
	extern SignatureAOI doAttack;

	// input
	extern SignatureAOI sdl_pollEvent_caller;
}

