#pragma once

#include <cstdint>      // uint8_t
#include <vector>

#include "Utility.hpp"

class Detour : public Util::NonCopyable {
public:
	Detour();
	Detour(Detour &&other);
	~Detour();
	auto operator=(Detour &&other) noexcept -> Detour&;

	auto install(uintptr_t insertion_addr,
				 int opcodes_len,
				 uintptr_t detour_addr) -> bool;
	auto isEnabled() const -> bool;
	auto remove() -> bool;

	auto opcodes_len() const -> size_t;
	auto getJmpBackAddr() const -> uintptr_t;

private:
	bool _enabled;
	uintptr_t _insertion_addr;
	std::vector<uint8_t> _old_opcodes;

	/// cool stuff:opcode(s) with a jump to desired function
	auto _overwriteOpcodesWithDetour(uintptr_t detour_addr) const -> void;
	auto _restoreOldOpcodes() const -> void;
};