#include "Bunnyhop.hpp"
#include "Pointers/GamePointerFactory.hpp"

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

void Bunnyhop::start() {
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

void Bunnyhop::stop() {
	if (m_enabled) {
		m_enabled = false;

		// prevent bhop not starting
		// when jump key is down when you land on the ground and bhop is enabled
		*m_jump = 4;
	}
}

void Bunnyhop::hook() {
	auto p_onGround_land = GamePointerFactory::get(op_onGround_inc());
    bool l_inc_detour_success = m_detour_onGround_inc.install<BUNNYHOP_DETOUR_LEN_ON_GROUND>(
            p_onGround_land,
            &Bunnyhop::hook_onGround_inc,
            this,
            DetourToMethod::CODE_BEFORE_DETOUR
    );

    if (!l_inc_detour_success) {
        Log::log("Bunnyhop failed to detour inc on_ground");
    }

	auto p_onGround_leave = GamePointerFactory::get(op_onGround_dec());
    bool l_dec_detour_success = m_detour_onGround_dec.install<BUNNYHOP_DETOUR_LEN_ON_GROUND>(
            p_onGround_leave,
            &Bunnyhop::hook_onGround_dec,
            this,
            DetourToMethod::CODE_BEFORE_DETOUR
    );

    if (!l_dec_detour_success) {
        Log::log("Bunnyhop failed to detour dec on_ground");
    }
}

void Bunnyhop::unhook() {
	if (!m_detour_onGround_inc.remove()) {
		Log::log("Bunnyhop failed to un-detour inc on_ground");
	}

	if (!m_detour_onGround_dec.remove()) {
		Log::log("Bunnyhop failed to un-detour dec on_ground");
	}
}

void Bunnyhop::hook_onGround_inc() {
	if (m_enabled) {
		*m_jump = 5;
	}
}

void Bunnyhop::hook_onGround_dec() {
	if (m_enabled) {
		*m_jump = 4;
	}
}
