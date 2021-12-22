#include "libEntry.hpp"

#include "Hack.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"
#include "MemoryUtils.hpp"

std::atomic_bool g_loaded{false};
std::mutex g_ejectingFromWithinGameMutex;
std::atomic_bool g_ejectingFromWithinGame{false};

#ifdef __linux__

#include <dlfcn.h>      // dlclose, dladdr
pthread_t g_nix_hack_thread;
pthread_t g_nix_eject_thread;

__attribute__((constructor))
auto initLibrary() -> void {

	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "initLibrary");
	nix_init_hack();
}

__attribute__((destructor))
auto exitLibrary() -> void {
	if (g_ejectingFromWithinGame) {
		if (pthread_join(g_nix_eject_thread, nullptr) != 0) {
			Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Join eject-thread failed");
		}
	} else {
		eject_hack(nullptr);
	}
	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "exited Library");
}

auto nix_hack_main(void *) -> void* {
	hack_loop();

	return nullptr;
}

auto nix_init_hack() -> void {
	if (g_loaded == false) {
		g_do_exit = false;

		// start hack in its own thread
		pthread_create(&g_nix_hack_thread, nullptr, &nix_hack_main, nullptr);
		pthread_setname_np(g_nix_hack_thread, "cssHack");

		g_loaded = true;
	}
}

auto eject_from_within_hack() -> void {
	std::scoped_lock l_lock(g_ejectingFromWithinGameMutex);
	if (!g_ejectingFromWithinGame) {
		g_do_exit = true;
		g_ejectingFromWithinGame = true;

		pthread_create(&g_nix_eject_thread, nullptr, &eject_hack, nullptr);
	}
}

auto eject_hack(void *) -> void* {
	// ensure hack was loaded before
	if (g_loaded == true) {
		g_loaded = false;
		// stop hack loop
		g_do_exit = true;

		// wait for hack termination
		if (pthread_join(g_nix_hack_thread, nullptr) != 0) {
			Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Join hack main failed");
		}
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
			std::scoped_lock l_lock(g_ejectingFromWithinGameMutex);
			pthread_attr_t tAttr;
			pthread_t closeThread;
			pthread_attr_init(&tAttr);
			pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
			pthread_create(&closeThread, &tAttr, (void *(*)(void *)) dlclose, handle);
		}
	}
	return nullptr;
}

#else // windows
#include <thread>
HMODULE g_win_hModule;
HANDLE g_win_hack_thread = INVALID_HANDLE_VALUE;
HANDLE g_win_eject_thread = INVALID_HANDLE_VALUE;

auto WINAPI DllMain(HMODULE hModule,
					DWORD  ul_reason_for_call,
					LPVOID) -> BOOL 
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "initLibrary");
	g_win_hModule = hModule;
	win_init_hack();
	break;

  case DLL_PROCESS_DETACH:
	// TODO: synchronize & remove atomic
	if (!g_ejectingFromWithinGame) {
	  // FIXME: thread lock is held in dllmain, no synchronizing is possible.
	  //        how to exit?
	  eject_from_libEntry();
	}
	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "exitLibrary");
	break;
  }
  return TRUE;
}

auto WINAPI win_hack_main(LPVOID) -> DWORD {
	try {
		hack_loop();
	}
	catch (std::exception & e) {
		Log::log<Log::FLUSH>("Exception: ", e.what(), "\nExiting.");
		if (!g_do_exit) {
			// user didn't wish for exit. Eject.
			eject_from_within_hack();
		}
	}

	return EXIT_SUCCESS;
}

auto win_init_hack() -> void {
  if (g_loaded == false) {
	g_loaded = true;
	g_do_exit = false;

	// start hack_main in its own thread
	g_win_hack_thread = CreateThread(nullptr, 0, &win_hack_main, nullptr, 0, nullptr);
	auto ret = SetThreadDescription(g_win_hack_thread, L"cssHack");
	if (FAILED(ret)) {
	  Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Set cssHack name failed");
	}
  }
}

auto WINAPI win_eject_hack(LPVOID) -> DWORD {
  // ensure hack was loaded before
  if (g_loaded == true) {
	g_loaded = false;
	// stop hack loop
	g_do_exit = true;

	// wait for hack termination
	auto ret = WaitForSingleObject(g_win_hack_thread, INFINITE);
	if (ret == WAIT_TIMEOUT) {
	  Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Join hack main timeout");
	} else if (ret == WAIT_FAILED) {
	  Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Join hack main failed");
	}

	// manually stop log, as its destructor won't be called on DLL exit itself, just on process exit
	  // not stopping will cause the log thread and thus the DLL to persist in the process
	Log::stop();

	if (g_ejectingFromWithinGame) {
	  FreeLibraryAndExitThread(g_win_hModule, 0);
	}
  }
  return EXIT_SUCCESS;
}

auto eject_from_within_hack() -> void {
  std::scoped_lock l_lock(g_ejectingFromWithinGameMutex);
  if (!g_ejectingFromWithinGame) {
	g_do_exit = true;
	g_ejectingFromWithinGame = true;

	g_win_eject_thread = CreateThread(nullptr, 0, &win_eject_hack, nullptr, 0, nullptr);

	auto ret = SetThreadDescription(g_win_eject_thread, L"ejectThread");
	if (FAILED(ret)) {
	  Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Set ejectThread name failed");
	}
  }
}

auto eject_from_libEntry() -> void {
  // TODO: join + destroy
  g_win_eject_thread = CreateThread(nullptr, 0, &win_eject_hack, nullptr, 0, nullptr);

  auto ret = SetThreadDescription(g_win_eject_thread, L"ejectThread");
  if (FAILED(ret)) {
	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Set ejectThread name failed");
  }
}
#endif
