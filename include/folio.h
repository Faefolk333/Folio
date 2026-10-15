#pragma once

#include <stdbool.h>
#include <stddef.h>

#define FOLIO_PATH_MAX 512
#define FOLIO_TITLE_MAX 96
#define FOLIO_MAX_CHAPTERS 128

typedef enum {
    FOLIO_FORMAT_TEXT,
    FOLIO_FORMAT_HTML,
    FOLIO_FORMAT_EPUB,
    FOLIO_FORMAT_PDF,
    FOLIO_FORMAT_UNKNOWN
} FolioFormat;

typedef struct {
    size_t offset;
    char title[80];
} FolioChapter;

typedef struct {
    char *text;                 /* UTF-8, owned by this document */
    size_t length;
    char title[FOLIO_TITLE_MAX];
    FolioChapter chapters[FOLIO_MAX_CHAPTERS];
    int chapterCount;
    FolioFormat format;
} FolioDocument;

FolioFormat folio_detect_format(const char *path);
bool folio_load_document(const char *path, FolioDocument *out, char *error, size_t errorSize);
void folio_free_document(FolioDocument *doc);
