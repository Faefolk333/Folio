#pragma once
#include "folio.h"
bool folio_load_epub(const char *path, FolioDocument *doc, char *error, size_t capacity);
char *folio_markup_text(const char *input, size_t length, size_t *outputLength);
