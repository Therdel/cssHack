//
// Created by therdel on 06.06.19.
//
#pragma once
#include <string>

/**
 * strongly typed module name
 */
class LibName {
public:
	explicit constexpr LibName(const char* name) noexcept
			: m_name(name) {}

	constexpr operator const char*() const noexcept {
		return m_name;
	}

	operator std::string() const {
		return {m_name};
	}

	std::string name() const {
		return *this;
	}
private:
	const char *m_name;
};

namespace libNames {

#ifdef __linux__
	constexpr LibName client{"client.so"};
	constexpr LibName materialsystem{"materialsystem.so"};
	constexpr LibName engine{"engine.so"};
	constexpr LibName vgui2{"vgui2.so"};
	constexpr LibName launcher{"launcher.so"};
	constexpr LibName libtogl{"libtogl.so"};
	constexpr LibName shaderapidx9{"shaderapidx9.so"};
#else
	constexpr LibName client{"client.dll"};
	constexpr LibName materialsystem{"materialsystem.dll"};
	constexpr LibName engine{"engine.dll"};
#endif
}
