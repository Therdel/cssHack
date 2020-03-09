//
// Created by therdel on 06.06.19.
//
#pragma once

#include <string_view>

namespace libNames {
#ifdef __linux__
	// TODO: Enum of std::string_views, name gotten by
	static constexpr std::string_view client = "client.so";
	static constexpr std::string_view materialsystem = "materialsystem.so";
	static constexpr std::string_view engine = "engine.so";
	static constexpr std::string_view vgui2 = "vgui2.so";
	static constexpr std::string_view launcher = "launcher.so";
	static constexpr std::string_view libtogl = "libtogl.so";
	static constexpr std::string_view shaderapidx9 = "shaderapidx9.so";
#else
	static constexpr std::string_view client = "client.dll";
	static constexpr std::string_view materialsystem = "materialsystem.dll";
	static constexpr std::string_view engine = "engine.dll";
	static constexpr std::string_view launcher = "launcher.dll";
	static constexpr std::string_view inputsystem = "inputsystem.dll";
#endif
}
