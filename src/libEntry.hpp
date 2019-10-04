#pragma once

#include <mutex>
#include <atomic>

// flag that tells if the hack was loaded before
extern std::atomic_bool g_loaded;
extern std::mutex g_ejectingFromWithinGameMutex;
extern std::atomic_bool g_ejectingFromWithinGame;

#ifdef __linux__

#include <pthread.h>

extern pthread_t g_nix_hack_thread;
extern pthread_t g_nix_eject_thread;

void *nix_hack_main(void *);

void nix_init_hack();

__attribute__((constructor))
void initLibrary();

__attribute__((destructor))
void exitLibrary();

void eject_from_within_hack();

void *eject_hack(void *);

#else // windows
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

extern HMODULE g_win_hModule;
extern HANDLE g_win_hack_thread;
extern HANDLE g_win_eject_thread;

DWORD WINAPI win_hack_main(LPVOID lpParam);

// start hack main in its own thread
// source: https://stackoverflow.com/questions/32252143/stdthread-cause-deadlock-in-dllmain
void win_init_hack();

BOOL WINAPI DllMain(HMODULE hModule,
					DWORD ul_reason_for_call,
					LPVOID);

DWORD WINAPI win_eject_hack(LPVOID lpParam);

void eject_from_within_hack();

void eject_from_libEntry();

#endif
