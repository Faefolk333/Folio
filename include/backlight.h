#pragma once
#include <stdbool.h>
bool backlight_sleep(bool sleeping);
void backlight_restore(void);
void backlight_resume(void);
void backlight_exit(void);
