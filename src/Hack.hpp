#pragma once

#define POLLS_PER_SECOND 100
#define POLL_SLEEP_MS (1000/POLLS_PER_SECOND)

auto hack_loop() -> void;
