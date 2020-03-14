//
// Created by therdel on 05.10.19.
//
#include "MemoryUtils.hpp"
#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"

auto MemoryUtils::MemoryRange::address() const -> uintptr_t {
    return reinterpret_cast<uintptr_t>(memoryRange.data());
}

using ScopedReProtect = MemoryUtils::ScopedReProtect;

ScopedReProtect::ScopedReProtect(MemoryRange oldProtection)
		: _active(true)
		, _oldProtection(oldProtection) {}

ScopedReProtect::ScopedReProtect(ScopedReProtect &&other) noexcept
		: _active(other._active)
		, _oldProtection(other._oldProtection) {
	other._active = false;
}

auto ScopedReProtect::operator=(ScopedReProtect &&other) noexcept -> ScopedReProtect& {
	_active = other._active;
	other._active = false;
	_oldProtection = other._oldProtection;
	return *this;
}

ScopedReProtect::~ScopedReProtect() {
	restore();
}

auto ScopedReProtect::restore() -> bool {
	if (_active) {
		auto result = set_memory_protection(_oldProtection);
		if (result.has_value()) {
			_active = false;
		}
	}

	return !_active;
}

auto MemoryUtils::lib_base_32(std::string_view libName) -> uintptr_t {
	auto l_libBase = lib_base_32_timeout(libName, (std::chrono::milliseconds::max) ());
	if (!l_libBase) {
		std::string error{libName};
		error += " base addr not found";
		Log::log<Log::FLUSH>(error);
		throw std::runtime_error(error);
	}
	return *l_libBase;
}

auto MemoryUtils::scoped_remove_memory_protection(uintptr_t address, size_t length) -> std::optional<ScopedReProtect> {
	auto *charAddress = reinterpret_cast<char*>(address);
	MemoryRange newProtection{ { charAddress, length}, MemoryRange::noProtection() };
	auto formerProtection = set_memory_protection(newProtection);

	if (formerProtection.has_value()) {
		return ScopedReProtect(*formerProtection);
	} else {
		return std::nullopt;
	}
}
