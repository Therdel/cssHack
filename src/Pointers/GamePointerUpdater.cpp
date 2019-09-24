//
// Created by therdel on 24.09.19.
//
#include "GamePointerUpdater.hpp"
#include "GamePointerFactory.hpp"

GamePointerUpdater::GamePointerUpdater()
: m_localplayer(GamePointerFactory::get(GamePointerDef::localplayer))
, m_detour_localplayer_update()
, m_detour_localplayer_invalidate() {
	hook();
}

GamePointerUpdater::~GamePointerUpdater() {
	unhook();
}

void GamePointerUpdater::hook() {
	bool l_detour_update_success = m_detour_localplayer_update.install<GAMEPOINTERUPDATER_DETOUR_LEN_ON_UPDATE>(
			GamePointerFactory::get(GamePointerDef::op_localplayer_update),
			&GamePointerUpdater::hookOnLocalplayerUpdate,
			this,
			DetourToMethod::CODE_BEFORE_DETOUR
	);

	if (!l_detour_update_success) {
		Log::log("GamePointerUpdater failed to detour localplayer update");
	}

	bool l_detour_invalidate_success = m_detour_localplayer_invalidate.install<GAMEPOINTERUPDATER_DETOUR_LEN_ON_INVALIDATE>(
			GamePointerFactory::get(GamePointerDef::op_localplayer_invalidate),
			&GamePointerUpdater::hookOnLocalplayerInvalidate,
			this,
			DetourToMethod::CODE_BEFORE_DETOUR
	);

	if (!l_detour_invalidate_success) {
		Log::log("GamePointerUpdater failed to detour localplayer invalidate");
	}
}

void GamePointerUpdater::unhook() {
	if (!m_detour_localplayer_update.remove()) {
		Log::log("GamePointerUpdater failed to un-detour localplayer update");
	}

	if (!m_detour_localplayer_invalidate.remove()) {
		Log::log("GamePointerUpdater failed to un-detour localplayer invalidate");
	}
}

void GamePointerUpdater::hookOnLocalplayerUpdate() {
	Log::log(Log::Channel::STD_OUT, "Localplayer update");
	m_localplayer.update();
}

void GamePointerUpdater::hookOnLocalplayerInvalidate() {
	Log::log(Log::Channel::STD_OUT, "Localplayer invalidate");
	m_localplayer.update();
}
