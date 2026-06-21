// FIXME: put here so MSVC recognizes std::optional
#ifndef __linux__
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#endif

#include <atomic>
#include <thread>
#include <chrono>

#include <SDL_events.h> // SDL_Event, SDL_KeyboardEvent

// own includes
#include "Hack.hpp"
#include "libEntry.hpp"
#include "Pointers/GameVars.hpp"
#include "Aimbot.hpp"
#include "Bunnyhop.hpp"
#include "Input.hpp"
#include "Visuals/DrawHook.hpp"
#include "Visuals/GUI.hpp"
#include "Visuals/ESP.hpp"
// #include "Visuals/Wallhack.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"

// std::atomic_bool g_do_exit{false};
auto exit_requested() -> bool {
	return g_lifecycle == Lifecycle::EJECTING_BY_OURSELF
	    || g_lifecycle == Lifecycle::EJECTING_EXTERNALLY;
}

using namespace std::chrono_literals;

static auto wait_for_inject_combination(Input &input) -> void {
	std::atomic_bool l_injected{false};
	auto keyHandler = ScopedKeyHandler(input,
	                                   key_inject, [&](SDL_KeyboardEvent const &) {
				l_injected = true;
				return true;
			});
	// while (l_injected == false && g_do_exit == false) {
	while (l_injected == false && !exit_requested()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(POLL_SLEEP_MS));
	}
}

static auto onEjectKey(SDL_KeyboardEvent const &event) -> bool {
	// Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "onEjectKey");
	if (event.repeat == 0) {
		if (event.type == SDL_KEYDOWN) {
			eject_from_within_hack();
		}
	}
	return false;
}

static auto onBhopKey(Bunnyhop &bhop, SDL_KeyboardEvent const &event) -> bool {
	if (event.repeat == 0) {
		if (event.type == SDL_KEYDOWN) {
			bhop.start();
		} else {
			bhop.stop();
		}
	}
	// hide event from game
	return true;
}

static auto onAimKey(Aimbot &aimbot, SDL_KeyboardEvent const &event) -> bool {
	if (event.repeat == 0) {
		if (event.type == SDL_KEYDOWN) {
			aimbot.startAim();
		} else {
			aimbot.stopAim();
		}
	}
	return false;
}

static auto onAimKey(Aimbot &aimbot, SDL_Event const &event) -> bool {
	bool stealEvent = false;
	if (event.type == SDL_MOUSEBUTTONDOWN ||
	    event.type == SDL_MOUSEBUTTONUP) {
		auto &buttonEvent = event.button;
		if (buttonEvent.button == SDL_BUTTON_LEFT) {
			if (buttonEvent.state == SDL_PRESSED) {
				aimbot.startAim();
			} else {
				aimbot.stopAim();
			}
			stealEvent = true;
		}
	}

	return stealEvent;
}

static auto onTriggerKey(Aimbot &aimbot, SDL_KeyboardEvent const &event) -> bool {
	if (event.repeat == 0) {
		if (event.type == SDL_KEYDOWN) {
			aimbot.start360(Aimbot::Mode360::TRIGGER, Aimbot::Mode360::TRIGGER);
		} else {
			aimbot.stop360();
			aimbot.stopAim();
		}
	}
	return false;
}

#include <format>
#include "Pointers/libNames.hpp"
#include "MemoryUtils.hpp"
// class CBaseHandle;
// class IClientEntity;
// class IClientNetworkable;
// class IClientUnknown;

// class IClientEntityList {
// 	public:
// 		virtual IClientNetworkable* GetClientNetworkable(int entindex) = 0;
// 		virtual IClientNetworkable* GetClientNetworkableFromHandle(CBaseHandle handle) = 0;
// 		virtual IClientUnknown* GetClientUnknownFromHandle(CBaseHandle handle) = 0;
// 		virtual IClientEntity* GetClientEntity(int entindex) = 0;
// 		virtual IClientEntity* GetClientEntityFromHandle(CBaseHandle handle) = 0;
// 		virtual int NumberOfEntities(bool include_non_networkable) = 0;
// 		virtual int GetHighestEntityIndex(void) = 0;
// 		virtual void SetMaxEntities(int max_entities) = 0;
// 		virtual int GetMaxEntities() = 0;
// };
auto hack_loop() -> void {
	// TODO: detect LD_PRELOAD method with wait
	/*
	// while (!g_do_exit) {
	while (!exit_requested()) {

	  bool isKeyEjectDown = GetAsyncKeyState(VK_F11) & 0x1;
	  if (isKeyEjectDown) {
		eject_from_within_hack();
	  }
	  std::this_thread::sleep_for(100ms);
	}
	*/

	std::this_thread::sleep_for(100ms); // FIXME: Crashes when put at main() beginning
	const GameVars gameVars = GameVars::scan();
	
	
	Input l_input{gameVars};
	l_input.setKeyHandler(key_eject, &onEjectKey);
	// std::this_thread::sleep_for(10000ms); // FIXME: Crashes when put at main() beginning
	// throw std::runtime_error("whoopsies, crash in hack_loop(), before end");
	// wait_for_inject_combination(l_input);



	// if (!exit_requested()) {
	// 	Log::log("Injected");
	// }

	// // source: https://github.com/aixxe/cstrike-basehook-linux/blob/master/src/Basehook.cpp#L60
	// #define VCLIENTENTITYLIST_INTERFACE_VERSION	"VClientEntityList003"

	// const std::optional<uintptr_t> addressCreateInterface = MemoryUtils::getSymbolAddress(libNames::client, "CreateInterface");
	// if (!addressCreateInterface) {
	// 	Log::log("Failed to find CreateInterface symbol in client library");
	// } else {
	// 	Log::log(std::format("Found CreateInterface symbol at {:#x}", *addressCreateInterface));
	// 	// get IClientEntityList interface

	// 	using CreateInterfaceFn = void* (*)(const char *pName, int *pReturnCode);
	// 	const auto createInterface = reinterpret_cast<CreateInterfaceFn>(*addressCreateInterface);
	// 	int returnCode = 420;
	// 	const auto clientEntityListRaw = createInterface(VCLIENTENTITYLIST_INTERFACE_VERSION, &returnCode);
	// 	if (clientEntityListRaw) {
	// 		Log::log(std::format("Found IClientEntityList at {:p}\nGameVars: {:p}", clientEntityListRaw, (void*)&gameVars.clientEntityList));
	// 	} else {
	// 		Log::log(std::format("Failed to get IClientEntityList via CreateInterface(\"{}\") with return code {}", VCLIENTENTITYLIST_INTERFACE_VERSION, returnCode));
	// 	}

	// 	auto clientEntityList = reinterpret_cast<IClientEntityList*>(clientEntityListRaw);
	// 	const auto highestEntityIndex = clientEntityList->GetHighestEntityIndex();
	// 	const auto numberOfEntities = clientEntityList->NumberOfEntities(false);
	// 	const auto numberOfEntitiesNN = clientEntityList->NumberOfEntities(true);
	// 	const auto maxEntities = clientEntityList->GetMaxEntities();
	// 	Log::log(std::format("IClientEntityList: HighestEntityIndex {}\nNumberOfEntities {}, NumberOfEntitiesNN {}\n, MaxEntities {}", highestEntityIndex, numberOfEntities, numberOfEntitiesNN, maxEntities));

	// 	int countClientEntities = 0;
	// 	for (int i=0; i<=highestEntityIndex; i++) {
	// 		auto entity = clientEntityList->GetClientEntity(i);
	// 		if (entity) {
	// 			countClientEntities++;
	// 			if (i < 6) {
	// 				const auto *player = reinterpret_cast<overlay_structs::LocalPlayer*>(entity);
	// 				Log::log<Log::FLUSH>(std::format("Found entity at index {}: {:p}\npBoneMatrix: {:p}", i, (void*)entity, (void*)player->pBoneMatrix.value));
	// 			}
	// 		}
	// 	}
	// 	int countClientNetworkables = 0;
	// 	for (int i=0; i<=highestEntityIndex; i++) {
	// 		auto entity = clientEntityList->GetClientNetworkable(i);
	// 		if (entity) {
	// 			countClientNetworkables++;
	// 		}
	// 	}
	// 	Log::log(std::format("IClientEntityList: Counted {} entities by GetClientEntity & {} entities by GetClientNetworkable", countClientEntities, countClientNetworkables));

	// 	for (int i=0; i<64; ++i) {
	// 		auto entity = clientEntityList->GetClientEntity(i);
	// 	}

	// 	// auto clientEntityList = reinterpret_cast<overlay_structs::IClientEntityList*>(createInterface(VCLIENTENTITYLIST_INTERFACE_VERSION, nullptr));
	// 	// if (clientEntityList) {
	// 	// 	Log::log(std::format("Found IClientEntityList at {:p}", clientEntityList));
	// 	// 	gameVars.setClientEntityList(clientEntityList);
	// 	// } else {
	// 	// 	Log::log(std::format("Failed to get IClientEntityList via CreateInterface(\"{}\")", VCLIENTENTITYLIST_INTERFACE_VERSION), Log::Channel::ERROR);
	// 	// }
	// }

	auto inGame = [&] { return !exit_requested() && gameVars.is_ingame == 1; };
	auto inPlay = [&] { return inGame() && gameVars.is_inmenu == 0; };

	while (!exit_requested()) {
		if (!inGame()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(POLL_SLEEP_MS));
			continue;
		}

		// initialize game hacks
		// TODO: test without hooks, to remove bugs that happen without the hooking / locking dependency to game threads
		struct WaitOnDestroy { ~WaitOnDestroy() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }};
		WaitOnDestroy wait;
		DrawHook l_drawHook{gameVars};
		GUI l_gui{l_drawHook, l_input};
		Aimbot l_aimbot{gameVars, l_gui};
		Bunnyhop l_bunnyhop{gameVars};
		ESP l_esp{gameVars, l_drawHook, l_gui, l_aimbot};
//		Wallhack l_wallhack;

		while (inGame()) {
			if (!inPlay()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(POLL_SLEEP_MS));
				continue;
			}

			// initialize input bindings
			ScopedKeyHandler bhopHandler(l_input,
			                             key_bhop,
			                             [&](SDL_KeyboardEvent const &event) {
				                             return onBhopKey(l_bunnyhop, event);
			                             });
			ScopedKeyHandler aimbotHandler(l_input,
			                               key_aim,
			                               [&](SDL_KeyboardEvent const &event) {
				                               return onAimKey(l_aimbot, event);
			                               });
			ScopedKeyHandler triggerBotHandler(l_input,
			                                   key_trigger,
			                                   [&](SDL_KeyboardEvent const &event) {
				                                   return onTriggerKey(l_aimbot, event);
			                                   });

			while (inPlay()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(POLL_SLEEP_MS));
			}
			// TODO: opening/closing ingame chat quickly will somehow get stuck in the loop
			// Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "!inPlay anymore");
			l_input.removeMouseHandler();
			l_bunnyhop.stop();
			l_aimbot.stopAim();
		}
		// Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "!inGame anymore");
	}
	// Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "exiting hack loop");

	throw std::runtime_error("hellooo"); // FIXME: crashes with external eject
}
