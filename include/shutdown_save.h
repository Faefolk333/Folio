#pragma once
#include <stdbool.h>
#include <stddef.h>

/* Single-use exit checkpoint. Private FS session; waits at most 500 ms for I/O.
   A timed-out worker owns its snapshot until process termination. */
bool shutdown_save_write(const char *path, const char *record, size_t length);
