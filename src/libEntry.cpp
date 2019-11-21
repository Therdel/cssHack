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
void initLibrary() {

	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "initLibrary");
	nix_init_hack();
}

__attribute__((destructor))
void exitLibrary() {
	if (g_ejectingFromWithinGame) {
		if (pthread_join(g_nix_eject_thread, nullptr) != 0) {
			Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Join eject-thread failed");
		}
	} else {
		eject_hack(nullptr);
	}
	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "exited Library");
}

void *nix_hack_main(void *) {
	hack_loop();

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

void eject_from_within_hack() {
	std::lock_guard<std::mutex> l_lock(g_ejectingFromWithinGameMutex);
	if (!g_ejectingFromWithinGame) {
		g_do_exit = true;
		g_ejectingFromWithinGame = true;

		pthread_create(&g_nix_eject_thread, nullptr, &eject_hack, nullptr);
	}
}

void *eject_hack(void *) {
	// ensure hack was loaded before
	if (g_loaded == true) {
		g_loaded = false;
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

#else // windows
#include <thread>
HMODULE g_win_hModule;
HANDLE g_win_hack_thread = INVALID_HANDLE_VALUE;
HANDLE g_win_eject_thread = INVALID_HANDLE_VALUE;

BOOL WINAPI DllMain(HMODULE hModule,
  DWORD  ul_reason_for_call,
  LPVOID)
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

DWORD WINAPI win_hack_main(LPVOID) {
  hack_loop();

  return EXIT_SUCCESS;
}

void win_init_hack() {
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

DWORD WINAPI win_eject_hack(LPVOID) {
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

void eject_from_within_hack() {
  std::lock_guard<std::mutex> l_lock(g_ejectingFromWithinGameMutex);
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

void eject_from_libEntry() {
  // TODO: join + destroy
  g_win_eject_thread = CreateThread(nullptr, 0, &win_eject_hack, nullptr, 0, nullptr);

  auto ret = SetThreadDescription(g_win_eject_thread, L"ejectThread");
  if (FAILED(ret)) {
	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Set ejectThread name failed");
  }
}
#endif
