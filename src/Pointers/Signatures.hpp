//
// Created by therdel on 12.10.19.
//
#pragma once

#include <array>
#include <string_view>

#include "libNames.hpp"

/// Byte pattern used to find a memory location.
/// Signatures are defined by a C-string describing the bytes that make up the wanted piece of memory.
/// Known bytes are specified in hex, variable/"don't care" bytes using '??'
///
/// example: "74 4E E8 ?? ?? ?? ??"
/// would match every byte array of length 7 with specified first 3 bytes.
struct Signature {
	constexpr Signature(std::string_view libName, std::string_view sig, ptrdiff_t aoi_offset, size_t aoi_length)
	: libName(libName)
	, sig(sig)
	, aoi_offset(aoi_offset)
	, aoi_length(aoi_length) {}

	std::string_view libName;
	std::string_view sig;
	ptrdiff_t aoi_offset;
	size_t aoi_length;
};

namespace Signatures {
#ifdef __linux__
	constexpr Signature onGround(libNames::client, "74 4E 83 05 ?? ?? ?? ?? 01", 4, 4);
	constexpr Signature onGround_op_land(libNames::client, "74 4E 83 05 ?? ?? ?? ?? 01", 2, 7);
	constexpr Signature onGround_op_leave(libNames::client, "8b 53 08 83 2d ?? ?? ?? ?? 01", 3, 7);
#else
#endif
}

