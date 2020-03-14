#include "Detour.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"
#include "MemoryUtils.hpp"

Detour::Detour()
: _enabled(false)
, _insertion_addr(0)
, _old_opcodes() {
}

Detour::Detour(Detour &&other)
: _enabled(other._enabled)
, _insertion_addr(other._insertion_addr)
, _old_opcodes(std::move(other._old_opcodes)) {
	// remove other detour, so he doesn't remove our hook
	other._enabled = false;
}

Detour::~Detour() {
	remove();
}

auto Detour::operator=(Detour &&other) noexcept -> Detour& {
	_enabled = other._enabled;
	_insertion_addr = other._insertion_addr;
	_old_opcodes = std::move(other._old_opcodes);

	// remove other detour, so he doesn't remove our hook
	other._enabled = false;

	return *this;
}

auto Detour::install(uintptr_t insertion_addr, int opcodes_len, uintptr_t detour_addr) -> bool {
	// we need at least 1 byte for the jump opcode and 4 for the detour function address
	if (opcodes_len < 5) {
		Log::log<Log::FLUSH>("Detour: not enough opcode bytes to detour");
		throw std::runtime_error("Detour: not enough opcode bytes to detour");
	}
	remove();

	_enabled = true;
	_insertion_addr = insertion_addr;

	// store old opcodes
	auto begin_it = reinterpret_cast<uint8_t*>(insertion_addr);
	auto end_it = reinterpret_cast<uint8_t*>(insertion_addr + opcodes_len);
	_old_opcodes = std::vector<uint8_t>(begin_it, end_it);

	// remove write protection at insertion point
	auto protect_change = MemoryUtils::scoped_remove_memory_protection(_insertion_addr, opcodes_len);
	if (!protect_change.has_value()) {
		// unable to change memory protection of code segment
		Log::log("Detour::install failed to add write permissions");
		return false;
	}

	_overwriteOpcodesWithDetour(detour_addr);

	if (!protect_change->restore()) {
		// restoring old memory protection failed
		Log::log("Detour::install failed to restore previous permissions");
		// restore old opcodes before failing
		// TODO re-use remove() code
		_restoreOldOpcodes();
		return false;
	}

	// initialize as working detour
	_enabled = true;

	return true;
}

auto Detour::isEnabled() const -> bool {
	return _enabled;
}

auto Detour::remove() -> bool {
	bool l_success = true;
	if (isEnabled()) {
		// remove write protection at insertion point
		auto protect_change = MemoryUtils::scoped_remove_memory_protection(_insertion_addr, opcodes_len());
		if (!protect_change.has_value()) {
			// unable to change memory protection of code segment
			Log::log("Detour::remove failed to add write permissions");
			return false;
		}

		_restoreOldOpcodes();

		if (!protect_change->restore()) {
			// restoring old memory protection failed
			Log::log("Detour::remove failed to restore previous permissions");
			// restore old opcodes before failing
			// TODO re-use remove() code
			_restoreOldOpcodes();
			l_success = false;
		}
		if (l_success) {
			_enabled = false;
		}
	}

	return true;
}

auto Detour::opcodes_len() const -> size_t {
	return _old_opcodes.size();
}

auto Detour::getJmpBackAddr() const -> uintptr_t {
	return _insertion_addr + opcodes_len();
}

/// cool stuff:opcode(s) with a jump to desired function
auto Detour::_overwriteOpcodesWithDetour(uintptr_t detour_addr) const -> void {
	// 1. erase old operation(s) - that is, replace with do-none-operands (0x90)
	auto begin_it = (uint8_t*)_insertion_addr;
	auto end_it = begin_it + opcodes_len();
	std::fill(begin_it, end_it, 0x90);

	// 2. write jump opcode
	*(uint8_t*)(_insertion_addr) = 0xE9;

	// 3. compute the relative jump address - that's the address difference
	//    from the op AFTER the resulting jump (1b jmp op + 4b jmp target address = 5b)
	//    to the target detour function address
	uintptr_t l_relative_address = detour_addr - (_insertion_addr + 5);

	// 4. write the relative jump address one byte after the jump opcode
	*(uintptr_t*)(_insertion_addr + 1) = l_relative_address;
}

auto Detour::_restoreOldOpcodes() const -> void {
	auto out_it = reinterpret_cast<uint8_t*>(_insertion_addr);
	std::copy(_old_opcodes.begin(), _old_opcodes.end(), out_it);
	// TODO: flush instruction cache
	//          http://man7.org/linux/man-pages/man2/cacheflush.2.html
	//          https://www.malwaretech.com/2015/01/inline-hooking-for-programmers-part-2.html
}