#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "folio.h"
typedef struct {uint32_t generation;int language;char folder[FOLIO_PATH_MAX];} FolioSettings;
bool settings_load(const char *root,FolioSettings *out);
bool settings_save(const char *root,FolioSettings *settings);
bool settings_folder_valid(const char *folder);
