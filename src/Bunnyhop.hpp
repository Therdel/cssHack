#pragma once

#include "Utility.hpp"
#include "DetourToCallback.hpp"
#include "Pointers/SharedGamePointer.hpp"

class Bunnyhop : public Util::NonCopyable, public Util::NonMovable {
public:
	Bunnyhop();

	// remove hooks
	~Bunnyhop();

	auto isEnabled() const -> bool {
		return m_enabled;
	}

	auto start() -> void;

	auto stop() -> void;

private:
	SharedGamePointer<int> m_on_ground;
	SharedGamePointer<int> m_jump;

	DetourToCallback m_detour_onGround_inc;
	DetourToCallback m_detour_onGround_dec;

	bool m_enabled;

	auto hook() -> void;

	auto unhook() -> void;

	// hook that gets called when the player lands on the ground
	// causes a +jump action
	auto hook_onGround_inc() -> void;

	// hook that gets called when the player leaves the ground
	// causes a -jump action
	auto hook_onGround_dec() -> void;
};