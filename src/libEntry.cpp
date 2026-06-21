#include "libEntry.hpp"

#include "Hack.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"
#include "MemoryUtils.hpp"

std::atomic<Lifecycle> g_lifecycle{Lifecycle::RUNNING};

#ifdef __linux__

#include <dlfcn.h>      // dlopen, dlclose
pthread_t g_nix_hack_thread;

__attribute__((constructor))
auto initLibrary() -> void {
	// Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "initLibrary");
	
	// start hack in its own thread
	pthread_create(&g_nix_hack_thread, nullptr, &nix_hack_main, nullptr);
	pthread_setname_np(g_nix_hack_thread, "cssHack");
}

__attribute__((destructor))
auto exitLibrary() -> void {
	auto expected = Lifecycle::RUNNING;
	g_lifecycle.compare_exchange_strong(expected, Lifecycle::EJECTING_EXTERNALLY);
	
	// wait for hack termination
	if (pthread_join(g_nix_hack_thread, nullptr) != 0) {
		Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Join hack main failed");
	}
	
	// TODO: unhook here?
	Log::stop();
}

auto nix_hack_main(void *) -> void* {
	try {
		hack_loop();
	}
	catch (std::exception & e) {
		const auto what = e.what();
		// FIXME: somehow deadlocks when exception happens...
		// Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Exception: ", e.what(), "\nExiting.");
		// TODO: debug with simulate_external_eject(). (not really, won't stop all threads right?)
		//       this is needed when Input is destroyed as part of hack_loop().

		// if we're not ejecting already, we must ensure self-ejection
		auto expected = Lifecycle::RUNNING;
		g_lifecycle.compare_exchange_strong(expected, Lifecycle::EJECTING_BY_OURSELF);
		
	}
	
	if (g_lifecycle == Lifecycle::EJECTING_BY_OURSELF) {
		const auto libPath = MemoryUtils::this_lib_path();

		// 1. get library handle (dlopen), incrementing the library handle's reference count by 1.
		// 2. close once (dlclose) to decrement/restore the reference count again
		// source: https://linux.die.net/man/3/dlclose
		void *handle = dlopen(libPath.c_str(),  RTLD_LAZY | RTLD_NOLOAD);
		dlclose(handle);

		// call dlclose in a separate thread so this library code can be
		// safely deallocated during dlclose
		pthread_t dlcloseThread;
		pthread_attr_t tAttr;
		pthread_attr_init(&tAttr);
		pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
		// run dlclose in a separate thread, which will run __attribute((destructor)), which joins this "main" thread.
		// After dlclose() finishes the destructor, it deallocates the library.
		// This is safe as no thread is running any of its code at that point.
		pthread_create(&dlcloseThread, &tAttr, (void *(*)(void *)) dlclose, handle);
	}

	return nullptr;
}

// TODO
auto simulate_external_eject() -> void {
	// // TODO: NOT HOW EXTERNAL EJECT WOULD WORK - EXTERNAL WOULD CAS IN DESTRUCTOR, NOT HERE!!
	// // 		only here to prevent multiple concurrent calls.
	// auto expected = Lifecycle::RUNNING;
	// const bool exchanged = g_lifecycle.compare_exchange_strong(expected, Lifecycle::EJECTING_EXTERNALLY);
	// if (!exchanged) {
	// 	// another thread already initiated eject, so we can just return
	// 	return;
	// }

	auto libPath = MemoryUtils::this_lib_path();
	void *handle = dlopen(libPath.c_str(),  RTLD_LAZY | RTLD_NOLOAD);
	dlclose(handle);

	pthread_attr_t tAttr;
	pthread_t dlcloseThread;
	pthread_attr_init(&tAttr);
	pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
	pthread_create(&dlcloseThread, &tAttr, (void *(*)(void *)) dlclose, handle);
}

auto eject_from_within_hack() -> void {
	auto expected = Lifecycle::RUNNING;
	const auto exchanged = g_lifecycle.compare_exchange_strong(
		expected,
		Lifecycle::EJECTING_BY_OURSELF);
	
	// TODO: why does this deadlock renderer in GUI::onDraw?
	//     - SDL_FreeCursor doesn't run, it deadlocks
	// simulate_external_eject();

	// if (exchanged) {
	// 	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "eject_from_within_hack()");
	// }
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
