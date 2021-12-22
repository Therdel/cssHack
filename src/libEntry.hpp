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

auto nix_hack_main(void *) -> void*;

auto nix_init_hack() -> void;

__attribute__((constructor))
auto initLibrary() -> void;

__attribute__((destructor))
auto exitLibrary() -> void;

auto eject_from_within_hack() -> void;

auto eject_hack(void *) -> void*;

#else // windows
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

extern HMODULE g_win_hModule;
extern HANDLE g_win_hack_thread;
extern HANDLE g_win_eject_thread;

auto WINAPI win_hack_main(LPVOID lpParam) -> DWORD;

// start hack main in its own thread
// source: https://stackoverflow.com/questions/32252143/stdthread-cause-deadlock-in-dllmain
auto win_init_hack() -> void;

auto WINAPI DllMain(HMODULE hModule,
					DWORD ul_reason_for_call,
					LPVOID) -> BOOL;

auto WINAPI win_eject_hack(LPVOID lpParam) -> DWORD;

auto eject_from_within_hack() -> void;

auto eject_from_libEntry() -> void;

#endif
