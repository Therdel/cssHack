#pragma once

#include "Utility.hpp"
#include "DetourToCallback.hpp"
#include "Pointers/SharedGamePointer.hpp"

class Bunnyhop : public Util::NonCopyable, public Util::NonMovable {
public:
	Bunnyhop();

	// remove hooks
	~Bunnyhop();

	bool isEnabled() const {
		return m_enabled;
	}

	void start();

	void stop();

private:
	SharedGamePointer<int> m_on_ground;
	SharedGamePointer<int> m_jump;

	DetourToCallback m_detour_onGround_inc;
	DetourToCallback m_detour_onGround_dec;

	bool m_enabled;

	void hook();

	void unhook();

	// hook that gets called when the player lands on the ground
	// causes a +jump action
	void hook_onGround_inc();

	// hook that gets called when the player leaves the ground
	// causes a -jump action
	void hook_onGround_dec();
};