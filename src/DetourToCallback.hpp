#pragma once

#include <optional>
#include <vector>
#include <functional>

#include "Utility.hpp"
#include "Detour.hpp"
#include "MemoryUtils.hpp"

struct SignatureAOI;

// stores all general purpose registers
// source http://sparksandflames.com/files/x86InstructionChart.html
struct PushAllRegisters {
	const uint8_t PUSH_EAX = 0x50;
	const uint8_t PUSH_ECX = 0x51;
	const uint8_t PUSH_EDX = 0x52;
	const uint8_t PUSH_EBX = 0x53;
	const uint8_t PUSH_ESI = 0x56;
	const uint8_t PUSH_EDI = 0x57;

	const uint8_t PUSHF = 0X9C;

	static void writeOpcodes(void *address) {
		new(address) PushAllRegisters;
	}
};

// restores all general purpose registers
// source http://sparksandflames.com/files/x86InstructionChart.html
struct PopAllRegisters {
	const uint8_t POPF = 0X9D;

	const uint8_t POP_EDI = 0x5F;
	const uint8_t POP_ESI = 0x5E;
	const uint8_t POP_EBX = 0x5B;
	const uint8_t POP_EDX = 0x5A;
	const uint8_t POP_ECX = 0x59;
	const uint8_t POP_EAX = 0x58;

	static void writeOpcodes(void *address) {
		new(address) PopAllRegisters;
	}
};

class DetourToCallback : public Util::NonCopyable, public Util::NonMovable {
public:
	using callback = std::function<void()>;

	enum OLD_CODE_EXEC {
		CODE_BEFORE_DETOUR,
		CODE_AFTER_DETOUR,
		NO_EXEC
	};

	DetourToCallback();
	~DetourToCallback();

	auto install(const SignatureAOI& signature,
				 callback callback,
				 OLD_CODE_EXEC policy = CODE_AFTER_DETOUR) -> bool;

	auto install(uintptr_t insertion_addr,
				 int opcodes_len,
				 callback callback,
				 OLD_CODE_EXEC policy = CODE_AFTER_DETOUR) -> bool;

	// returns true for the detour was successfully disabled or already disabled before
	auto remove() -> bool;
	auto isEnabled() const -> bool;

private:
	enum MethodCallingConvention {
		push_on_stack,
		pass_by_ecx
	};

	bool _enabled;
	callback _callback;
	std::vector<uint8_t> _trampolineCodeBuf;
	std::optional<MemoryUtils::ScopedReProtect> _scopedTrampolineProtection;
	Detour _detourToTrampoline;

	// raw address of a non-static method
	template<typename Class>
	static auto _rawMethodAddress(void (Class::* method)(void) const) -> uintptr_t;

	// build assembly code that calls object.callbackMethod() at runtime
	auto _buildTrampoline(size_t opcodes_len,
						  uintptr_t insertion_addr,
						  OLD_CODE_EXEC policy) -> void;
};

// raw address of a non-static callbackMethod
template<typename Class>
auto DetourToCallback::_rawMethodAddress(void (Class::* method)() const) -> uintptr_t {
	// clearly insane magic to get MSVC to hand the addresses of member functions
	// https://stackoverflow.com/questions/8121320/get-memory-address-of-member-function
	// simplified by
	// https://www.codeproject.com/Questions/1032379/how-to-print-the-address-of-a-member-function-usin
	uintptr_t rawAddress = reinterpret_cast<uintptr_t&>(method);
	return rawAddress;
}
