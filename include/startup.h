#pragma once
#include <stdbool.h>
/* Uses CPU text consoles until startup_use_gpu() hands the screens to citro2d. */
void startup_begin(void);
bool startup_stage(const char *message);
void startup_log(const char *message);
void startup_error(const char *message);
void startup_use_gpu(void);
