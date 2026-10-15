#pragma once
#include <stdbool.h>
bool home_close_start(void);
void home_close_suspend(bool suspended);
/* Claims normal teardown, or waits for an already-issued system close. */
void home_close_stop(void);
/* Called only after the event loop ends, before GPU cleanup. */
void home_close_finish(void);
