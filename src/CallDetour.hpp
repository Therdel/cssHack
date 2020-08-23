//
// Created by therdel on 23.08.20.
//
#pragma once

#include "MemoryUtils.hpp"
#include "Utility.hpp"

/// \brief Redirects a call assembly operation.
///        Actions are reverted upon destruction.
class CallDetour final : public Util::NonCopyable {
public:
	/// \param addr_call_opcode Address of call operation byte
	/// \param addr_new_target Address of replacement function
	CallDetour(Util::Address addr_call_opcode, Util::Address addr_new_target)
	: _addr_call_opcode{ addr_call_opcode }
	, _old_call_target_offset{ _redirect_call(addr_call_opcode, addr_new_target) } {
	}

	~CallDetour() {
		_restore_opcodes();
	}

private:
	Util::Address _addr_call_opcode;
	Util::Offset _old_call_target_offset;

	auto _redirect_call(Util::Address addr_call_opcode, Util::Address addr_new_target) const -> Util::Offset {
		// calculate call-relative address to target
		Util::Address addr_next_op = addr_call_opcode + Util::Offset{ 0x05 };
		Util::Offset offset_next_op_to_target = addr_new_target - addr_next_op;
		Util::Address addr_call_target_offset = addr_call_opcode + Util::Offset{ 0x01 };

		auto old_call_target_offset = *addr_call_target_offset.as_pointer<Util::Offset>();

		// patch this shit
		auto scoped_permission = MemoryUtils::scoped_remove_memory_protection(addr_call_target_offset, 4);
		*addr_call_target_offset.as_pointer<Util::Offset>() = offset_next_op_to_target;

		return old_call_target_offset;
	}

	auto _restore_opcodes() const -> void {
		auto scoped_permission = MemoryUtils::scoped_remove_memory_protection(_old_call_target_offset, 4);
		Util::Address addr_call_target_offset = _addr_call_opcode + Util::Offset{ 0x01 };
		*addr_call_target_offset.as_pointer<Util::Offset>() = _old_call_target_offset;
	}
};
