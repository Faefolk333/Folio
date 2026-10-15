#pragma once
#include "settings.h"
#include <stddef.h>
bool settings_job_start(const FolioSettings *value);
bool settings_job_busy(void);
/* 0 pending/idle, 1 saved, -1 failed; releases a completed worker. */
int settings_job_poll(FolioSettings *saved);
unsigned settings_job_stage(void);
int32_t settings_job_error(void);
void settings_job_shutdown(void);
/* Native record I/O: used only from the private settings worker. */
bool settings_io_active(void);
bool settings_io_read(const char *path,void *data,size_t size);
bool settings_io_write(const char *path,const void *data,size_t size);
