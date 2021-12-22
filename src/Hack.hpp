#pragma once

#include <atomic>

#define POLLS_PER_SECOND 100
#define POLL_SLEEP_MS (1000/POLLS_PER_SECOND)

// flag that tells if the hack should be terminated
extern std::atomic_bool g_do_exit;

auto hack_loop() -> void;
