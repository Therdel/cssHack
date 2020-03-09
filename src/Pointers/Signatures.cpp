#include "Signatures.hpp"
#include "libNames.hpp"

namespace Signatures {
	#ifdef __linux__
	SignatureAOI onGround(libNames::client, { "74 4E 83 05 ?? ?? ?? ?? 01" }, 4, 4);
	SignatureAOI onGround_op_land(libNames::client, { "74 4E 83 05 ?? ?? ?? ?? 01" }, 2, 7);
	SignatureAOI onGround_op_leave(libNames::client, { "8b 53 08 83 2d ?? ?? ?? ?? 01" }, 3, 7);
	#else
	SignatureAOI onGround(libNames::client, { "85 FF 74 5C FF 05 ?? ?? ?? ?? 85 DB 74 0D" }, 6, 4);
	SignatureAOI onGround_op_land(libNames::client, { "85 FF 74 5C FF 05 ?? ?? ?? ?? 85 DB 74 0D" }, 4, 6);
	SignatureAOI onGround_op_leave(libNames::client, { "FF 0D ?? ?? ?? ?? E8 45 3F 15 00" }, 0, 6);
	#endif
}