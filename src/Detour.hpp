#pragma once

#include <type_traits>  // enable_if_t
#include <cstdint>      // uint8_t
#include <optional>
#include <array>

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"
#include "MemoryUtils.hpp"
#include "Utility.hpp"

class Detour : public Utility::NonCopyable {
public:
	Detour()
			: m_enabled(false)
			, m_insertion_addr(0)
			, m_old_opcodes() {
	}

	Detour(Detour &&other)
			: m_enabled(other.m_enabled)
			, m_insertion_addr(other.m_insertion_addr)
			, m_old_opcodes(std::move(other.m_old_opcodes)) {
		// remove other detour, so he doesn't remove our hook
		other.m_enabled = false;
	}

	~Detour() {
		remove();
	}

	Detour &operator=(Detour &&other) noexcept {
		m_enabled = other.m_enabled;
		m_insertion_addr = other.m_insertion_addr;
		m_old_opcodes = std::move(other.m_old_opcodes);

		// remove other detour, so he doesn't remove our hook
		other.m_enabled = false;

		return *this;
	}

	// TODO: clarify SFINAE
	template<size_t opcodes_len,
			// we need at least 1 byte for the jump opcode and 4 for the detour function address
			typename std::enable_if_t<(opcodes_len >= 5), int> = 0>
	bool install(uintptr_t insertion_addr, uintptr_t detour_addr) {
		remove();

		m_enabled = true;
		m_insertion_addr = insertion_addr;

		// store old opcodes
		auto begin_it = reinterpret_cast<uint8_t *>(insertion_addr);
		auto end_it = reinterpret_cast<uint8_t *>(insertion_addr + opcodes_len);
		m_old_opcodes = std::vector<uint8_t>(begin_it, end_it);

		// remove write protection at insertion point
		auto protect_change = MemoryUtils::scoped_remove_memory_protection(m_insertion_addr, opcodes_len);
		if (!protect_change.has_value()) {
			// unable to change memory protection of code segment
			Log::log("Detour::install failed to add write permissions");
			return false;
		}

		overwriteOpcodesWithDetour(detour_addr);

		if (!protect_change->restore()) {
			// restoring old memory protection failed
			Log::log("Detour::install failed to restore previous permissions");
			// restore old opcodes before failing
			// TODO re-use remove() code
			restoreOldOpcodes();
			return false;
		}

		// initialize as working detour
		m_enabled = true;

		return true;
	}

	bool isEnabled() const {
		return m_enabled;
	}

	bool remove() {
		bool l_success = true;
		if (isEnabled()) {
			// remove write protection at insertion point
			auto protect_change = MemoryUtils::scoped_remove_memory_protection(m_insertion_addr, opcodes_len());
			if (!protect_change.has_value()) {
				// unable to change memory protection of code segment
				Log::log("Detour::remove failed to add write permissions");
				return false;
			}

			restoreOldOpcodes();

			if (!protect_change->restore()) {
				// restoring old memory protection failed
				Log::log("Detour::remove failed to restore previous permissions");
				// restore old opcodes before failing
				// TODO re-use remove() code
				restoreOldOpcodes();
				l_success = false;
			}
			if (l_success) {
				m_enabled = false;
			}
		}

		return true;
	}

	size_t opcodes_len() const {
		return m_old_opcodes.size();
	}

	uintptr_t getJmpBackAddr() const {
		return m_insertion_addr + opcodes_len();
	}

private:
	bool m_enabled;
	uintptr_t m_insertion_addr;
	std::vector<uint8_t> m_old_opcodes;

	/// cool stuff:opcode(s) with a jump to desired function
	void overwriteOpcodesWithDetour(uintptr_t detour_addr) const {
		// 1. erase old operation(s) - that is, replace with do-none-operands (0x90)
		auto begin_it = (uint8_t*)m_insertion_addr;
		auto end_it = begin_it+opcodes_len();
		std::fill(begin_it, end_it, 0x90);

		// 2. write jump opcode
		*(uint8_t *) (m_insertion_addr) = 0xE9;

		// 3. compute the relative jump address - that's the address difference
		//    from the op AFTER the resulting jump (1b jmp op + 4b jmp target address = 5b)
		//    to the target detour function address
		uintptr_t l_relative_address = detour_addr - (m_insertion_addr + 5);

		// 4. write the relative jump address one byte after the jump opcode
		*(uintptr_t *) (m_insertion_addr + 1) = l_relative_address;
	}

	void restoreOldOpcodes() const {
		auto out_it = reinterpret_cast<uint8_t*>(m_insertion_addr);
		std::copy(m_old_opcodes.begin(), m_old_opcodes.end(), out_it);
		// TODO: flush instruction cache
		//          http://man7.org/linux/man-pages/man2/cacheflush.2.html
		//          https://www.malwaretech.com/2015/01/inline-hooking-for-programmers-part-2.html
	}
};