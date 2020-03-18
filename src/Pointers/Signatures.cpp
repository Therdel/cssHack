#include "Signatures.hpp"
#include "libNames.hpp"

using namespace libNames;

namespace Signatures {
	// bunnyhop
#ifdef __linux__
	SignatureAOI onGround_op_land(client, { "83 05 ?? ?? ?? ?? 01 85 FF 74 3B 8B 07 89 3C 24" }, 0, 7);
	SignatureAOI onGround_op_leave(client, { "83 2D ?? ?? ?? ?? 01 89 50 08 8B 43 08 8B 53 04" }, 0, 7);
	SignatureAOI onGround(client, onGround_op_leave.signature, 2, 4);
	SignatureAOI doJump(client, {"8B 3D ?? ?? ?? ?? 89 DA 83 CA 02 F7 C7 03 00 00 00 0F 45 DA"}, 2, 4);

	SignatureAOI aimAngles_x_op_read(engine, {"8D 74 26 00 55 8B 15 ?? ?? ?? ?? 89 E5  8B 45 0C 89 10"}, 5, 6);
	SignatureAOI aimAngles(engine, aimAngles_x_op_read.signature, 7, 4);
	// sig points to last instruction where vis angles are written to.
	SignatureAOI aimAnglesVisual_update(client, {"E8 ?? ?? ?? ?? F3 0F 10 06 F3 0F 58 83 28 0E 00 00 F3 0F 11 06 F3 0F 10 46 04 A1 ?? ?? ?? ?? ?? ?? ?? ?? 2C 0E 00 00 F3 0F 11 46 04 F3 0F 10 46 08 F3 0F 58 83 30 0E 00 00 F3 0F 11 46 08", }, 57, 5);
	SignatureAOI localplayer_base(client, { "8B 15 ?? ?? ?? ?? 85 D2 74 11 8B 02 89 14 24 FF 90" }, 2, 4);
	SignatureAOI playerArray_base(client, {"8B 15 ?? ?? ?? ?? 89 C3 31 C0 85 D2 74 24 8B 02"}, 2, 4);
	SignatureAOI playerPos(engine, {"F3 0F 10 9B E8 00 00 00 F3 0F 5C 2D ?? ?? ?? ??"}, 12, 4);
	SignatureAOI doAttack_op_read(client, {"8B 3D ?? ?? ?? ?? 89 DA 83 CA 01 F7 C7 03 00 00 00"}, 0, 6);
	SignatureAOI doAttack(client, doAttack_op_read.signature, 2, 4);

#else
	SignatureAOI onGround_op_land(client, { "85 FF 74 5C FF 05 ?? ?? ?? ?? 85 DB 74 0D" }, 4, 6);
	SignatureAOI onGround_op_leave(client, { "FF 0D ?? ?? ?? ?? E8 45 3F 15 00" }, 0, 6);
	SignatureAOI onGround(client, onGround_op_leave.signature, 2, 4);
	SignatureAOI doJump(client, { "74 08 23 CA 89 0D ?? ?? ?? ?? 8B 0D ?? ?? ?? ?? F6 C1 03 74 03 83 CE 08" }, 6, 4);

	// aimbot
	SignatureAOI aimAngles_x_op_read(engine, { "F3 0F 10 05 ?? ?? ?? ?? F3 0F 11 00 F3 0F 10 05 ?? ?? ?? ?? F3 0F 11 40 04 F3 0F 10 05 ?? ?? ?? ?? F3 0F 11 40 08 5D C2 04 00 B8" }, 0, 8);
	SignatureAOI aimAngles(engine, aimAngles_x_op_read.signature, 4, 4);
	SignatureAOI aimAnglesVisual_update(client, { "8B 5D 0C D9 00 D9 1B D9 40 04 D9 5B 04 D9 40 08 D9 5B 08" }, 3, 16);
	SignatureAOI localplayer_base(client, { "33 C0 39 0D ?? ?? ?? ?? 0F 94 C0 C3" }, 4, 4);
	SignatureAOI playerArray_base(client, { "8B 0D ?? ?? ?? ?? 8B F0 85 C9 74 33 8B 11" }, 2, 4);
	SignatureAOI playerPos(engine, { "?? ?? ?? ?? 9F F6 C4 44 7A 2D F3 0F 10 47 04" }, 0, 4);
	SignatureAOI doAttack_op_read(client, { "8B 0D ?? ?? ?? ?? F6 C1 03 74 03 83 CE 01 A8 01" }, 0, 6);
	SignatureAOI doAttack(client, doAttack_op_read.signature, 2, 4);

	// input
	SignatureAOI sdl_pollEvent_caller(inputsystem, { "E8 ?? ?? ?? ?? 83 C4 04 85 C0 74 18 8B FF 83 FE 64 7D 11" }, 0, 5);
#endif
}