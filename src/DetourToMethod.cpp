#include "DetourToMethod.hpp"

#include <cstring>  // std::memcpy

#include "Utility.hpp"
#include "Detour.hpp"
#include "MemoryScanner/MemoryScanner.hpp"

DetourToMethod::DetourToMethod()
: _enabled(false)
, _callback{ []{} }
, _trampolineCodeBuf()
, _scopedTrampolineProtection()
, _detourToTrampoline() {
}

// returns true for the detour was successfully disabled or already disabled before
auto DetourToMethod::remove() -> bool {
	bool l_success = true;

	if (isEnabled()) {
		// remove detour to trampoline
		// TODO:    1. flag detour as to-remove,
		//          2. remove detourToTrampoline on last trampoline call,
		//          3. restore permissions
		//          http://man7.org/linux/man-pages/man2/cacheflush.2.html
		//          https://www.malwaretech.com/2015/01/inline-hooking-for-programmers-part-2.html
		l_success &= _detourToTrampoline.remove();

		// TODO implement wait before removing execution rights?
		// this will crash when remove is called
		// while the trampoline/handler is still being executed

		// remove trampoline execution rights
		if (!_scopedTrampolineProtection->restore()) {
			// restoring old memory protection failed
			Log::log("DetourToMethod: Failed to restore"
				"previous permissions on trampoline");
			l_success = false;
		}

		_enabled = false;
	}
	return l_success;
}

auto DetourToMethod::isEnabled() const -> bool {
	return _enabled;
}

DetourToMethod::~DetourToMethod() {
	remove();
}

auto DetourToMethod::install(const SignatureAOI& signature,
							 callback callback,
							 OLD_CODE_EXEC policy) -> bool {
	uintptr_t insertion_addr = MemoryScanner::scanSignatureExpectOneResult(signature);
	int opcodes_len = signature.aoi_length;

	return install(insertion_addr,
				   opcodes_len,
				   std::move(callback),
				   policy);
}

// TODO enable for const methods too
// TODO great documentation required for this beautiful piece of greatness
// emphasize that the handling callbackMethod should NOT take too much time.
// it's possible that while the handling object gets deallocated, a thread of the detoured
// process is still executing the handling methods code.
// this is by principle thread-UNSAFE behavior in a thread-safety demanding environment...
// to minimize the risk of undefined behavior, handling methods should thus execute _quickly_
auto DetourToMethod::install(uintptr_t insertion_addr,
							 int opcodes_len,
							 callback callback,
							 OLD_CODE_EXEC policy) -> bool {
	// we need at least 1 byte for the jump opcode and 4 for the detour function address
	if (opcodes_len < 5) {
		Log::log<Log::FLUSH>("DetourToMethod: not enough opcode bytes to detour");
		throw std::runtime_error("DetourToMethod: not enough opcode bytes to detour");
	}
	// remove current detour, if present
	remove();

	_callback = std::move(callback);
	_enabled = true;
	_buildTrampoline(opcodes_len, insertion_addr, policy);

	// make our buffer executable - write access is also crucial
	// TODO? RESEARCH why do we need write access
	_scopedTrampolineProtection = MemoryUtils::scoped_remove_memory_protection(
		(uintptr_t)_trampolineCodeBuf.data(),
		_trampolineCodeBuf.size());
	if (!_scopedTrampolineProtection.has_value()) {
		// unable to change memory protection of code segment
		Log::log("DetourToMethod::install: Failed to enable execute permissions on trampoline");
		return false;
	}

	// install detour to our trampoline
	bool l_success = _detourToTrampoline.install(insertion_addr,
		opcodes_len,
		(uintptr_t)_trampolineCodeBuf.data());
	if (!l_success) {
		Log::log("DetourToMethod: Failed to detour to trampoline");

		// restore old trampoline permissions before failing
		// TODO reuse remove code
		if (!_scopedTrampolineProtection->restore()) {
			// restoring old memory protection failed
			Log::log("DetourToMethod: Failed to restore "
				"previous permissions on trampoline");
		}
		return false;
	}

	return true;
}

// ?somewhat? from: https://stackoverflow.com/questions/14346576/calling-c-member-function-with-reference-argument-from-asm
auto DetourToMethod::_buildTrampoline(size_t opcodes_len,
									  uintptr_t insertion_addr,
									  OLD_CODE_EXEC policy) -> void {
	uintptr_t callbackMethodAddress = _rawMethodAddress(&Callback::operator());
	uintptr_t callbackInstanceAddress = reinterpret_cast<uintptr_t>(&_callback);

	const uint8_t NOP = 0x90;
	const size_t l_trampolineSize = 21 + opcodes_len + sizeof(PushAllRegisters) + sizeof(PopAllRegisters);
	_trampolineCodeBuf = std::vector(l_trampolineSize, NOP);
	size_t nextIdx = 0;

	auto& codeBuf = _trampolineCodeBuf;

#ifdef __linux__
	MethodCallingConvention callingConvention = push_on_stack;
#else // windows
	MethodCallingConvention callingConvention = pass_by_ecx;
#endif

	// if policy demands CODE_BEFORE_DETOUR
	// the by detour overwritten code will be inserted before the handler call
	if (policy == CODE_BEFORE_DETOUR) {
		std::memcpy(&codeBuf[nextIdx], (void*)insertion_addr, opcodes_len);
		nextIdx += opcodes_len;
	}

	// save all general-purpose registers
	PushAllRegisters::writeOpcodes(&codeBuf[nextIdx]);
	nextIdx += sizeof(PushAllRegisters);

	// call member function in x86 linux assembly
	// adapted from: https://stackoverflow.com/questions/11602036/using-thunks-to-go-from-cdecl-to-thiscall-linux-x86
	// modification: no push/pop of return address, since the trampoline wasn't called, but jumped to
	// this leaves no necessity of tweaking any return addresses at this point

	if (callingConvention == push_on_stack) {
		// push object-pointer on stack
		// 68 EFBEADDE      - push DEADBEEF (DEADBEEF examplifies an actual address)
		codeBuf[nextIdx++] = 0x68; // push
	}
	else { // windows
		// pass object-pointer on ecx register
		// B9 EFBEADDE      - mov ecx, DEADBEEF (DEADBEEF examplifies an actual address)
		codeBuf[nextIdx++] = 0xB9; // mov ecx, ...
	}
	*(uintptr_t*)&codeBuf[nextIdx] = callbackInstanceAddress;
	nextIdx += 4;

	// call callbackMethod by address relative operation after call instruction
	// E8 EFBEADDE    - call[DEADBEEF] call relative to next instruction
	codeBuf[nextIdx++] = 0xE8;
	uintptr_t method_relative_address = callbackMethodAddress - (uintptr_t)&codeBuf[nextIdx + 4];
	*(uintptr_t*)&codeBuf[nextIdx] = method_relative_address;
	nextIdx += 4;

	if (callingConvention == push_on_stack) {
		// pop object-pointer - increment stack pointer by size of this ptr
		// 83 C4 08         - add esp, 04
		codeBuf[nextIdx++] = 0x83;
		codeBuf[nextIdx++] = 0xC4;
		codeBuf[nextIdx++] = 0x04;
	}

	// restore all general-purpose registers
	PopAllRegisters::writeOpcodes(&codeBuf[nextIdx]);
	nextIdx += sizeof(PopAllRegisters);

	// CODE_AFTER_DETOUR old code insertion point
	if (policy == CODE_AFTER_DETOUR) {
		std::memcpy(&codeBuf[nextIdx], (void*)insertion_addr, opcodes_len);
		nextIdx += opcodes_len;
	}

	// jump back to the instruction after the detour that calls the trampoline
	// E9 EFBEADDE    - jmp DEADBEEF
	codeBuf[nextIdx++] = 0xE9;
	uintptr_t l_addr_of_codeBuf_end = (uintptr_t)&codeBuf[nextIdx + 4];
	uintptr_t l_addr_of_op_after_orig_code = insertion_addr + opcodes_len;
	uintptr_t jump_back_to_orig_code_relative_address = l_addr_of_op_after_orig_code - l_addr_of_codeBuf_end;
	// TODO document better
	*(uintptr_t*)&codeBuf[nextIdx] = jump_back_to_orig_code_relative_address;
	nextIdx += 4; // unnecessary, but complete when I decide to add more stuff in the future
}