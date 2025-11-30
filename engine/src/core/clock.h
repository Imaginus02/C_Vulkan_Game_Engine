#pragma once
#include "defines.h"

typedef struct clock {
	f64 start_time;
	f64 elapsed;
} clock;

// Updates the provided clock. Should be called  just before checking elpased time
// Has no effect on non-started clocks.
KAPI void clock_update(clock* clock);

// Starts the provided clock. Resets the elpased time.
KAPI void clock_start(clock* clock);

// Stops the provided clock. Does not reset the elpased time.
KAPI void clock_stop(clock* clock);