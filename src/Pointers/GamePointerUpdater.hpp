//
// Created by therdel on 24.09.19.
//
#pragma once

#include "SharedGamePointer.hpp"
#include "../DetourToMethod.hpp"

#ifdef __linux__
#define GAMEPOINTERUPDATER_DETOUR_LEN_ON_UPDATE 6
#define GAMEPOINTERUPDATER_DETOUR_LEN_ON_INVALIDATE 0xA
#else
#endif

/// hooks into game code to capture changes and trigger updates on game pointers
class GamePointerUpdater {
public:
	GamePointerUpdater();
	~GamePointerUpdater();
private:
	SharedGamePointer<uintptr_t> m_localplayer;
	DetourToMethod m_detour_localplayer_update;
	DetourToMethod m_detour_localplayer_invalidate;

	void hook();
	void unhook();

	void hookOnLocalplayerUpdate();
	void hookOnLocalplayerInvalidate();
};