#pragma once

#include "Utility.hpp"
#include "DetourToMethod.hpp"
#include "Pointers/SharedGamePointer.hpp"

class Bunnyhop : public Utility::NonCopyable, public Utility::NonMovable {
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

	DetourToMethod m_detour_onGround_inc;
	DetourToMethod m_detour_onGround_dec;

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