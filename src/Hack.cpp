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
#include "Aimbot.hpp"
#include "Bunnyhop.hpp"
#include "Input.hpp"
#include "Visuals/DrawHook.hpp"
#include "Visuals/GUI.hpp"
#include "Visuals/ESP.hpp"
#include "Visuals/Wallhack.hpp"
#include "Pointers/GamePointerFactory.hpp"
#include "Pointers/GamePointerUpdater.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"

std::atomic_bool g_do_exit{false};

using namespace std::chrono_literals;

static void wait_for_inject_combination(Input &input) {
	std::atomic_bool l_injected{false};
	auto keyHandler = ScopedKeyHandler(input,
	                                   key_inject, [&](SDL_KeyboardEvent const &) {
				l_injected = true;
				return true;
			});
	while (l_injected == false && g_do_exit == false) {
		std::this_thread::sleep_for(std::chrono::milliseconds(POLL_SLEEP_MS));
	}
}

static bool onEjectKey(SDL_KeyboardEvent const &event) {
	if (event.repeat == 0) {
		if (event.type == SDL_KEYDOWN) {
			eject_from_within_hack();
		}
	}
	return false;
}

static bool onBhopKey(Bunnyhop &bhop, SDL_KeyboardEvent const &event) {
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

static bool onAimKey(Aimbot &aimbot, SDL_KeyboardEvent const &event) {
	if (event.repeat == 0) {
		if (event.type == SDL_KEYDOWN) {
			aimbot.startAim();
		} else {
			aimbot.stopAim();
		}
	}
	return false;
}

static bool onAimKey(Aimbot &aimbot, SDL_Event const &event) {
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

static bool onTriggerKey(Aimbot &aimbot, SDL_KeyboardEvent const &event) {
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

static bool g_acceptInputInGameMenus = true;

void hack_loop() {
	// TODO: detect LD_PRELOAD method with wait
	/*
	while (!g_do_exit) {

	  bool isKeyEjectDown = GetAsyncKeyState(VK_F11) & 0x1;
	  if (isKeyEjectDown) {
		eject_from_within_hack();
	  }
	  std::this_thread::sleep_for(100ms);
	}
	*/
	Input l_input;
	l_input.setKeyHandler(key_eject, &onEjectKey);
	wait_for_inject_combination(l_input);

	if (!g_do_exit) {
		Log::log("Injected");
	}

	while (g_do_exit == false) {
		// fixme: Crashes with nullptr on create when put at main() beginning
		auto l_isInGame = GamePointerFactory::get(GamePointerDef::isIngame());
		auto l_isInMenu = GamePointerFactory::get(GamePointerDef::isInMenu());

		if (g_do_exit == false && *l_isInGame == 1) {
			// initialize game hacks
			// fixme: Crashes when put at main() beginning
			// fixme: Sometimes an update after an invalidate is missing and causes a null deref
			GamePointerUpdater l_gamePointerUpdater;
			DrawHook l_drawHook;
			GUI l_gui(l_drawHook, l_input);
			l_gui.registerButton({"Update localplayer",
			                      [sp_localplayer = GamePointerFactory::get(GamePointerDef::localplayer())]
					                      () mutable {
				                      sp_localplayer.update();
			                      }});
			Aimbot l_aimbot(l_gui);
			Bunnyhop l_bunnyhop;
			ESP l_esp(l_drawHook, l_gui, l_aimbot);
//			Wallhack l_wallhack;

			while (g_do_exit == false && *l_isInGame == 1) {
				if (g_do_exit == false && *l_isInGame == 1 &&
				    (g_acceptInputInGameMenus || *l_isInMenu == 0)) {
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

					ScopedKeyHandler aim360Handler(l_input,
					                               key_trigger,
					                               [&](SDL_KeyboardEvent const &event) {
						                               return onTriggerKey(l_aimbot, event);
					                               });
					while (g_do_exit == false && *l_isInGame == 1 &&
					       (g_acceptInputInGameMenus || *l_isInMenu == 0)) {
						// sleep for some time to not kill performance
						std::this_thread::sleep_for(std::chrono::milliseconds(POLL_SLEEP_MS));
					}
					l_input.removeMouseHandler();
					l_bunnyhop.stop();
					l_aimbot.stopAim();
				}
				// sleep for some time to not kill performance
				std::this_thread::sleep_for(std::chrono::milliseconds(POLL_SLEEP_MS));
			}
		}
		// check if the player is ingame
		std::this_thread::sleep_for(std::chrono::milliseconds(POLL_SLEEP_MS));
	}

}
