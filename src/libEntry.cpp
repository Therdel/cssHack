#include "Hack.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"


#ifdef __linux__

__attribute__((constructor))
void initLibrary() {

	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "initLibrary");
	nix_init_hack();
}

__attribute__((destructor))
void exitLibrary() {
	if (g_ejectingFromWithinGame) {
		if (pthread_join(g_eject_thread, nullptr) != 0) {
			Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "Join eject-thread failed");
		}
	} else {
		eject_hack(nullptr);
	}
	Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "exited Library");
}

#else // windows
//BOOL WINAPI DllMain(
BOOL __declspec (dllexport) __stdcall DllMain(
		HMODULE hModule,
		DWORD  ul_reason_for_call,
		LPVOID)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:

	  Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "initLibrary");
			win_init_hack(hModule);
			break;
		case DLL_PROCESS_DETACH:
			eject_hack();
	  Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "exitLibrary");
			break;
		default: { }
	}
	return TRUE;
}

#endif