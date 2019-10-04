#include <string>
#include <thread>
#include <atomic>
#include <chrono>

// own includes
#include "Hack.hpp"
#include "MemoryUtils.hpp"
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

pthread_t g_nix_hack_thread;
// flag that tells if the hack was loaded before
std::atomic_bool g_loaded{false};

// flag that tells if the hack should be terminated
std::atomic_bool g_do_exit{false};

std::mutex g_ejectingFromWithinGameMutex{};
std::atomic_bool g_ejectingFromWithinGame{false};
pthread_t g_eject_thread;

using namespace std::chrono_literals;

void wait_for_inject_combination(Input &input) {
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

bool onEjectKey(SDL_KeyboardEvent const &event) {
	if (event.repeat == 0) {
		if (event.type == SDL_KEYDOWN) {
			eject_from_within_hack();
		}
	}
	return false;
}

bool onBhopKey(Bunnyhop &bhop, SDL_KeyboardEvent const &event) {
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

bool onAimKey(Aimbot &aimbot, SDL_KeyboardEvent const &event) {
	if (event.repeat == 0) {
		if (event.type == SDL_KEYDOWN) {
			aimbot.startAim();
		} else {
			aimbot.stopAim();
		}
	}
	return false;
}

bool onAimKey(Aimbot &aimbot, SDL_Event const &event) {
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

bool onTriggerKey(Aimbot &aimbot, SDL_KeyboardEvent const &event) {
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

bool g_acceptInputInGameMenus = true;

void hack_loop() {
	// TODO: detect LD_PRELOAD method with wait
	Input l_input;
	l_input.setKeyHandler(key_eject, &onEjectKey);
	wait_for_inject_combination(l_input);

	if (g_do_exit) {
		return;
	}
	Log::log("Injected");
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

#ifdef __linux__

#include <pthread.h>
#include <dlfcn.h>      // dlclose, dladdr

void *nix_hack_main(void *) {
	hack_loop();

	g_loaded = false;

	return nullptr;
}

void nix_init_hack() {
	if (g_loaded == false) {
		g_do_exit = false;

		// start hack in its own thread
		pthread_create(&g_nix_hack_thread, nullptr, &nix_hack_main, nullptr);
		pthread_setname_np(g_nix_hack_thread, "cssHack");

		g_loaded = true;
	}
}

#else

DWORD WINAPI win_hack_main(LPVOID lpParam) {

  Log::log("H4x0rd");

  hack_loop();

  g_loaded = false;
// manually stop log, as its destructor won't be called on DLL exit itself, just on process exit
  // not stopping will cause the log thread and thus the DLL to persist in the process
  Log::stop();

  FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);

  return EXIT_SUCCESS;
}

void win_init_hack(HMODULE hModule) {
	if (g_loaded == false) {
		g_do_exit = false;

		// start hack_main in its own thread
		g_win_hack_thread = CreateThread(nullptr, 0, &win_hack_main, hModule, 0, nullptr);

		g_loaded = true;
	}
}
#endif

void eject_from_within_hack() {
	std::lock_guard<std::mutex> l_lock(g_ejectingFromWithinGameMutex);
	if (!g_ejectingFromWithinGame) {
		g_do_exit = true;
		g_ejectingFromWithinGame = true;

		pthread_create(&g_eject_thread, nullptr, &eject_hack, nullptr);
	}
}

void *eject_hack(void *) {
	// ensure hack was loaded before
	if (g_loaded == true) {
		// stop hack loop
		g_do_exit = true;

		// wait for hack termination
#ifdef __linux__
		if (pthread_join(g_nix_hack_thread, nullptr) != 0) {
			Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Join hack main failed");
		}
#endif
		Log::stop();

		if (g_ejectingFromWithinGame) {
			auto libPath = MemoryUtils::this_lib_path();

			// get library handle and make it re-loadable again (RTLD_NOLOAD | RTLD_GLOBAL)
			// incrementing the reference count by 1
			// source: https://linux.die.net/man/3/dlclose
			void *handle = dlopen(libPath.c_str(), 6);
			// close once to decrease lib ref count because of dlopen a moment ago
			dlclose(handle);

			// call dlclose in a thread so this library code can be
			// safely deallocated during dlclose
			std::lock_guard<std::mutex> l_lock(g_ejectingFromWithinGameMutex);
			pthread_attr_t tAttr;
			pthread_t closeThread;
			pthread_attr_init(&tAttr);
			pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
			pthread_create(&closeThread, &tAttr, (void *(*)(void *)) dlclose, handle);
		}
	}
	return nullptr;
}
