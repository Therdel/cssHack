//
// Created by therdel on 05.10.19.
//
#include "MemoryUtils.hpp"
#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"

using ScopedReProtect = MemoryUtils::ScopedReProtect;

ScopedReProtect::ScopedReProtect(MemoryProtection oldProtection)
		: m_active(true)
		, m_formerProtection(oldProtection) {}

ScopedReProtect::ScopedReProtect(ScopedReProtect &&other) noexcept
		: m_active(other.m_active)
		, m_formerProtection(other.m_formerProtection) {
	other.m_active = false;
}

ScopedReProtect &ScopedReProtect::operator=(ScopedReProtect &&other) noexcept {
	m_active = other.m_active;
	other.m_active = false;
	m_formerProtection = other.m_formerProtection;
	return *this;
}

bool ScopedReProtect::restore() {
	if (m_active) {
		auto result = set_memory_protection(m_formerProtection);
		if (result.has_value()) {
			m_active = false;
		}
	}

	return !m_active;
}

uintptr_t MemoryUtils::lib_base_32(std::string_view libName) {
	auto l_libBase = lib_base_32_timeout(libName, (std::chrono::milliseconds::max) ());
	if (!l_libBase) {
		std::string error{libName};
		error += " base addr not found";
		Log::log<Log::FLUSH>(error);
		throw std::runtime_error(error);
	}
	return *l_libBase;
}

std::optional<MemoryUtils::ScopedReProtect>
MemoryUtils::scoped_remove_memory_protection(uintptr_t address, size_t length) {
	MemoryProtection newProtection{address, length, MemoryProtection::noProtection()};
	auto formerProtection = set_memory_protection(newProtection);

	if (formerProtection.has_value()) {
		return ScopedReProtect(*formerProtection);
	} else {
		return std::nullopt;
	}
}
