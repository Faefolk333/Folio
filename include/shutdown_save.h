#pragma once
#include <stdbool.h>
#include <stddef.h>

/* Bounded checkpoint. Private FS session; waits at most 500 ms for I/O.
   Reusable after completion; a timed-out worker retains its snapshot and
   disables further requests until process termination. Main thread only. */
bool shutdown_save_write(const char *path, const char *record, size_t length);
