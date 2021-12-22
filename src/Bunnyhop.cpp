#include "Bunnyhop.hpp"
#include "Pointers/GamePointerFactory.hpp"
#include "Pointers/Signatures.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX
#include "Log.hpp"

using namespace GamePointerDef;

Bunnyhop::Bunnyhop()
		: m_on_ground(GamePointerFactory::get(onGround()))
		, m_jump(GamePointerFactory::get(doJump()))
		, m_detour_onGround_inc()
		, m_detour_onGround_dec()
		, m_enabled(false) {
	hook();
}

Bunnyhop::~Bunnyhop() {
	stop();
	unhook();
}

auto Bunnyhop::start() -> void {
 	if (!m_enabled) {
		// the fake jumps are only triggered by landing on the ground, not while being on ground
		// so if we're currently standing on the ground...
		if (*m_on_ground != 0) {
			// we're manually faking the first jump
			*m_jump = 5;
		}

		m_enabled = true;
	}
}

auto Bunnyhop::stop() -> void {
	if (m_enabled) {
		m_enabled = false;

		// prevent bhop not starting
		// when jump key is down when you land on the ground and bhop is enabled
		*m_jump = 4;
	}
}

auto Bunnyhop::hook() -> void {
	bool l_inc_detour_success = m_detour_onGround_inc.install(
		Signatures::onGround_op_land,
		[this] { hook_onGround_inc(); },
		DetourToCallback::CODE_BEFORE_DETOUR);

    if (!l_inc_detour_success) {
        Log::log("Bunnyhop failed to detour inc on_ground");
    }

	bool l_dec_detour_success = m_detour_onGround_dec.install(
		Signatures::onGround_op_leave,
		[this] { hook_onGround_dec(); },
		DetourToCallback::CODE_BEFORE_DETOUR);

    if (!l_dec_detour_success) {
        Log::log("Bunnyhop failed to detour dec on_ground");
    }
}

auto Bunnyhop::unhook() -> void {
	if (!m_detour_onGround_inc.remove()) {
		Log::log("Bunnyhop failed to un-detour inc on_ground");
	}

	if (!m_detour_onGround_dec.remove()) {
		Log::log("Bunnyhop failed to un-detour dec on_ground");
	}
}

auto Bunnyhop::hook_onGround_inc() -> void {
	if (m_enabled) {
		*m_jump = 5;
	}
}

auto Bunnyhop::hook_onGround_dec() -> void {
	if (m_enabled) {
		*m_jump = 4;
	}
}
