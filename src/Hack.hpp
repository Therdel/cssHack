#pragma once
#include <atomic>
#include <mutex>

#define POLLS_PER_SECOND 100
#define POLL_SLEEP_MS (1000/POLLS_PER_SECOND)

extern pthread_t g_nix_hack_thread;
// flag that tells if the hack was loaded before
extern std::atomic_bool g_loaded;

// flag that tells if the hack should be terminated
extern std::atomic_bool g_do_exit;

extern std::mutex g_ejectingFromWithinGameMutex;
extern std::atomic_bool g_ejectingFromWithinGame;
extern pthread_t g_eject_thread;


void hack_loop();

#ifdef __linux__
#include <pthread.h>
void *nix_hack_main(void *);

void nix_init_hack();

#else
// windows includes
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

static HANDLE g_win_hack_thread = INVALID_HANDLE_VALUE;

DWORD WINAPI win_hack_main(LPVOID lpParam);

// start hack main in its own thread
// actual thread will start start some time AFTER dllmain finishes
void win_init_hack(HMODULE hModule);
#endif

/// <summary>Returns true if the current application has focus, false otherwise</summary>
// from https://stackoverflow.com/questions/7162834/determine-if-current-application-is-activated-has-focus

void eject_from_within_hack();
void* eject_hack(void *);