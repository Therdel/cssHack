#include "Signatures.hpp"
#include "libNames.hpp"

using namespace libNames;

namespace Signatures {
#ifdef __linux__
	SignatureAOI onGround_op_land(client, { "74 4E 83 05 ?? ?? ?? ?? 01" }, 2, 7);
	SignatureAOI onGround_op_leave(client, { "8b 53 08 83 2d ?? ?? ?? ?? 01" }, 3, 7);
	SignatureAOI onGround(client, onGround_op_leave.signature, 5, 4);
#else
	SignatureAOI onGround_op_land(client, { "85 FF 74 5C FF 05 ?? ?? ?? ?? 85 DB 74 0D" }, 4, 6);
	SignatureAOI onGround_op_leave(client, { "FF 0D ?? ?? ?? ?? E8 45 3F 15 00" }, 0, 6);
	SignatureAOI onGround(client, onGround_op_leave.signature, 2, 4);
	SignatureAOI doJump(client, { "74 08 23 CA 89 0D ?? ?? ?? ?? 8B 0D ?? ?? ?? ?? F6 C1 03 74 03 83 CE 08" }, 6, 4);

#endif
}