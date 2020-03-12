#pragma once

#include <optional>
#include <vector>
#include <cstring>  // std::memcpy

#include "Utility.hpp"
#include "Detour.hpp"
#include "MemoryScanner/MemoryScanner.hpp"

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

class DetourToMethod : public Utility::NonCopyable, public Utility::NonMovable {
public:
	enum OLD_CODE_EXEC {
		CODE_BEFORE_DETOUR,
		CODE_AFTER_DETOUR,
		NO_EXEC
	};

	enum MethodCallingConvention {
		push_on_stack,
		pass_by_ecx
	};

	DetourToMethod()
			: m_enabled(false)
			, m_object_ptr(0)
			, m_method_ptr(0)
			// allocate space for trampoline assembly code
			// TODO move length of code buf out of here?
			, m_trampolineCodeBuf()
			, m_scoped_trampoline_protection()
			, m_detourToTrampoline() {
	}

	/*
	// should disable this as this leads to errors on movable detoured-to-types,
	// which don't update the trampolines this-ptr on move
	DetourToMethod(DetourToMethod<opcodes_len> &&other)
			: m_enabled(other.m_enabled)
			, m_object_ptr(other.m_object_ptr)
			, m_method_ptr(other.m_method_ptr)
			, m_trampolineCodeBuf(std::move(other.m_trampolineCodeBuf))
			, m_scoped_trampoline_protection(std::move(other.m_scoped_trampoline_protection))
			, m_detourToTrampoline(std::move(other.m_detourToTrampoline)) {
		other.m_enabled = false;
	}

	DetourToMethod &operator=(DetourToMethod<opcodes_len> &&other) {
		m_enabled = other.m_enabled;
		m_object_ptr = other.m_object_ptr;
		m_method_ptr = other.m_method_ptr;
		m_trampolineCodeBuf = std::move(other.m_trampolineCodeBuf);
		m_scoped_trampoline_protection(std::move(other.m_scoped_trampoline_protection));
		m_detourToTrampoline = std::move(other.m_detourToTrampoline);

		other.m_enabled = false;

		return *this;
	}
	 */

	// TODO enable for const methods too
	// TODO great documentation required for this beautiful piece of greatness
	// emphasize that the handling method should NOT take too much time.
	// it's possible that while the handling object gets deallocated, a thread of the detoured
	// process is still executing the handling methods code.
	// this is by principle thread-UNSAFE behavior in a thread-safety demanding environment...
	// to minimize the risk of undefined behavior, handling methods should thus execute _quickly_
	template<typename HandlerClass>
	bool install(uintptr_t insertion_addr,
				 int opcodes_len,
	             void (HandlerClass::*pMethod)(void),
	             HandlerClass *pObject,
	             OLD_CODE_EXEC policy = CODE_AFTER_DETOUR) {
		// we need at least 1 byte for the jump opcode and 4 for the detour function address
		if (opcodes_len < 5) {
			Log::log<Log::FLUSH>("DetourToMethod: not enough opcode bytes to detour");
			throw std::runtime_error("DetourToMethod: not enough opcode bytes to detour");
		}
		// remove current detour, if present
		remove();

		m_enabled = true;
		// get the raw address of pMethod, using...
		// clearly insane magic to get MSVC to hand the addresses of member functions
		// https://stackoverflow.com/questions/8121320/get-memory-address-of-member-function
		// simplified by
		// https://www.codeprojectcheat.com/Questions/1032379/how-to-print-the-address-of-a-member-function-usin
		m_method_ptr = (uintptr_t &) pMethod;
		m_object_ptr = (uintptr_t) pObject;

		buildTrampoline(opcodes_len, insertion_addr, policy);

		// make our buffer executable - write access is also crucial
		// TODO? RESEARCH why do we need write access
		m_scoped_trampoline_protection = MemoryUtils::scoped_remove_memory_protection(
				(uintptr_t) m_trampolineCodeBuf.data(),
				m_trampolineCodeBuf.size());
		if (!m_scoped_trampoline_protection.has_value()) {
			// unable to change memory protection of code segment
			Log::log("DetourToMethod::install: Failed to enable execute permissions on trampoline");
			return false;
		}

		// install detour to our trampoline
		bool l_success = m_detourToTrampoline.install(insertion_addr,
													  opcodes_len,
													  (uintptr_t) m_trampolineCodeBuf.data());
		if (!l_success) {
			Log::log("DetourToMethod: Failed to detour to trampoline");

			// restore old trampoline permissions before failing
			// TODO reuse remove code
			if (!m_scoped_trampoline_protection->restore()) {
				// restoring old memory protection failed
				Log::log("DetourToMethod: Failed to restore "
				         "previous permissions on trampoline");
			}
			return false;
		}

		return true;
	}

	template<typename HandlerClass>
	bool install(const SignatureAOI& signature,
				 void (HandlerClass::* pMethod)(void),
				 HandlerClass* pObject,
				 OLD_CODE_EXEC policy = CODE_AFTER_DETOUR) {
		uintptr_t insertion_addr = MemoryScanner::scanSignatureExpectOneResult(signature);
		int opcodes_len = signature.aoi_length;
		return install(insertion_addr,
					   opcodes_len,
					   pMethod,
					   pObject,
					   policy);
	}

	// returns true for the detour was successfully disabled or already disabled before
	bool remove() {
		bool l_success = true;

		if (isEnabled()) {
			// remove detour to trampoline
			// TODO:    1. flag detour as to-remove,
			//          2. remove detourToTrampoline on last trampoline call,
			//          3. restore permissions
			//          http://man7.org/linux/man-pages/man2/cacheflush.2.html
			//          https://www.malwaretech.com/2015/01/inline-hooking-for-programmers-part-2.html
			l_success &= m_detourToTrampoline.remove();

			// TODO implement wait before removing execution rights?
			// this will crash when remove is called
			// while the trampoline/handler is still being executed

			// remove trampoline execution rights
			if (!m_scoped_trampoline_protection->restore()) {
				// restoring old memory protection failed
				Log::log("DetourToMethod: Failed to restore"
				         "previous permissions on trampoline");
				l_success = false;
			}

			m_enabled = false;
		}
		return l_success;
	}

	bool isEnabled() const {
		return m_enabled;
	}

	~DetourToMethod() {
		remove();
	}

private:
	bool m_enabled;
	// TODO? instead of uintptr_t, use pointers with class templates?
	uintptr_t m_object_ptr;
	uintptr_t m_method_ptr;
	// must be a heap-allocated buffer so the detour points to the trampoline code location after a move
	std::vector<uint8_t> m_trampolineCodeBuf;
	std::optional<MemoryUtils::ScopedReProtect> m_scoped_trampoline_protection;
	Detour m_detourToTrampoline;

	// do runtime code assembly that calls object.method()
	// ?somewhat? from: https://stackoverflow.com/questions/14346576/calling-c-member-function-with-reference-argument-from-asm
	void buildTrampoline(size_t opcodes_len,
						 uintptr_t insertion_addr,
						 OLD_CODE_EXEC policy) {
		const uint8_t NOP = 0x90;
		const size_t l_trampolineSize = 21 + opcodes_len + sizeof(PushAllRegisters) + sizeof(PopAllRegisters);
		m_trampolineCodeBuf = std::vector(l_trampolineSize, NOP);
		size_t nextIdx = 0;

		auto &codeBuf = m_trampolineCodeBuf;

#ifdef __linux__
		MethodCallingConvention callingConvention = push_on_stack;
#else // windows
		MethodCallingConvention callingConvention = pass_by_ecx;
#endif

		// if policy demands CODE_BEFORE_DETOUR
		// the by detour overwritten code will be inserted before the handler call
		if (policy == CODE_BEFORE_DETOUR) {
			std::memcpy(&codeBuf[nextIdx], (void *) insertion_addr, opcodes_len);
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
			*(uintptr_t *) &codeBuf[nextIdx] = m_object_ptr;
			nextIdx += 4;
		}
		else { // windows
			// pass object-pointer on ecx register
			// B9 EFBEADDE      - mov ecx, DEADBEEF (DEADBEEF examplifies an actual address)
			codeBuf[nextIdx++] = 0xB9; // mov ecx, ...
			*(uintptr_t*)&codeBuf[nextIdx] = m_object_ptr;
			nextIdx += 4;
		}

		// call method by address relative operation after call instruction
		// E8 EFBEADDE    - call[DEADBEEF] call relative to next instruction
		codeBuf[nextIdx++] = 0xE8;
		uintptr_t method_relative_address = m_method_ptr - (uintptr_t) &codeBuf[nextIdx + 4];
		*(uintptr_t *) &codeBuf[nextIdx] = method_relative_address;
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
			std::memcpy(&codeBuf[nextIdx], (void *) insertion_addr, opcodes_len);
			nextIdx += opcodes_len;
		}

		// jump back to the instruction after the detour that calls the trampoline
		// E9 EFBEADDE    - jmp DEADBEEF
		codeBuf[nextIdx++] = 0xE9;
		uintptr_t l_addr_of_codeBuf_end = (uintptr_t) &codeBuf[nextIdx + 4];
		uintptr_t l_addr_of_op_after_orig_code = insertion_addr + opcodes_len;
		uintptr_t jump_back_to_orig_code_relative_address = l_addr_of_op_after_orig_code - l_addr_of_codeBuf_end;
		// TODO document better
		*(uintptr_t *) &codeBuf[nextIdx] = jump_back_to_orig_code_relative_address;
		nextIdx += 4; // unnecessary, but complete when I decide to add more stuff in the future
	}
};