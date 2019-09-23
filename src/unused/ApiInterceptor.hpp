//
// Created by therdel on 11.07.19.
//
#pragma once

#include <vector>
#include <cstdint>
#include <cstring>      // std::memcpy

#include "../Detour.hpp"

template<// we need at least 1 byte for the jump opcode and 4 for the detour function address
		size_t opcodes_len,
		typename _Res,
		typename... _ArgTypes>
class ApiInterceptor;

template<// we need at least 1 byte for the jump opcode and 4 for the detour function address
		size_t opcodes_len,
		typename _Res,
		typename... _ArgTypes>
class ApiInterceptor<opcodes_len, _Res(_ArgTypes...)> {
	static_assert(opcodes_len >= 5);
public:
	using function_pointer = _Res(*)(_ArgTypes...);
private:
	const function_pointer m_orig_function_pointer;

	std::vector<uint8_t> m_orig_function_trampoline;
	std::optional<MemoryUtils::ScopedReProtect> m_scoped_trampoline_protection;
	Detour<opcodes_len> m_detour;

public:
	explicit ApiInterceptor(function_pointer original)
			: m_orig_function_pointer(original)
			, m_orig_function_trampoline(opcodes_len, 0x90) {
		std::memcpy((void *) m_orig_function_trampoline.data(),
		            (void *) original,
		            opcodes_len);

		// TODO: dirty hack time, works for SDL_PollEvent ONLY LOL HAHAHA
		std::memset((void*)&m_orig_function_trampoline[4],
					0x90, 5);
		m_orig_function_trampoline[4] = 0x8B;
		m_orig_function_trampoline[5] = 0x1C;
		m_orig_function_trampoline[6] = 0x24;


		// make our buffer executable - write access is also crucial
		// TODO? RESEARCH why do we need write access
		m_scoped_trampoline_protection = MemoryUtils::scoped_remove_memory_protection(
				(uintptr_t) m_orig_function_trampoline.data(),
				m_orig_function_trampoline.size());
		if (!m_scoped_trampoline_protection.has_value()) {
			// unable to change memory protection of code segment
			Log::log("ApiInterceptor: Failed to enable execute permissions on code buf");
			throw std::runtime_error("ApiInterceptor: Failed to enable execute permissions on code buf");
		}
	}

	bool interceptTo(function_pointer interceptor) {
		auto addr_interceptor = reinterpret_cast<uintptr_t>(interceptor);
		auto addr_functionOrig = reinterpret_cast<uintptr_t>(m_orig_function_pointer);
		return m_detour.install(addr_functionOrig, addr_interceptor);
	}

	void disable() {
		m_detour.disable();
	}

	~ApiInterceptor() {
		disable();

		// TODO implement wait before removing execution rights?
		// this will crash when disable is called
		// while the trampoline/handler is still being executed

		// remove trampoline execution rights
		if (!m_scoped_trampoline_protection->restore()) {
			// restoring old memory protection failed
			Log::log("ApiInterceptor: Failed to restore previous permissions on trampoline");
		}
	}

	function_pointer getTrampoline() const {
		return reinterpret_cast<function_pointer>(m_orig_function_trampoline.data());
	}

	_Res callTrampoline(_ArgTypes &&... args) const {
		std::cout << "callTramp" << std::endl;
		function_pointer orig_pointer = getTrampoline();
		return orig_pointer(std::forward<_ArgTypes>(args)...);
	}
};