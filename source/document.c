#include "folio.h"
#include "epub.h"
#include "encoding.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static bool ends_with(const char *s, const char *ext) {
    size_t a = strlen(s), b = strlen(ext);
    return a >= b && strcasecmp(s + a - b, ext) == 0;
}

FolioFormat folio_detect_format(const char *path) {
    if (ends_with(path, ".txt") || ends_with(path, ".md")) return FOLIO_FORMAT_TEXT;
    if (ends_with(path, ".html") || ends_with(path, ".htm")) return FOLIO_FORMAT_HTML;
    if (ends_with(path, ".epub")) return FOLIO_FORMAT_EPUB;
    if (ends_with(path, ".pdf")) return FOLIO_FORMAT_PDF;
    return FOLIO_FORMAT_UNKNOWN;
}

static void derive_title(const char *path, char *out, size_t cap) {
    const char *base = strrchr(path, '/'); base = base ? base + 1 : path;
    snprintf(out, cap, "%s", base);
    char *dot = strrchr(out, '.'); if (dot) *dot = 0;
}

static char *read_file(const char *path, size_t *size) {
    FILE *f = fopen(path, "rb"); if (!f) return NULL;
    fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f);
    if (n < 0 || n > 4 * 1024 * 1024) { fclose(f); return NULL; }
    char *data = malloc((size_t)n + 1); if (!data) { fclose(f); return NULL; }
    size_t got = fread(data, 1, (size_t)n, f); fclose(f); data[got] = 0; *size = got;
    return data;
}


bool folio_load_document(const char *path, FolioDocument *out, char *error, size_t errorSize) {
    memset(out, 0, sizeof(*out)); out->format = folio_detect_format(path); derive_title(path, out->title, sizeof(out->title));
    if (out->format == FOLIO_FORMAT_EPUB) return folio_load_epub(path,out,error,errorSize);
    if (out->format == FOLIO_FORMAT_PDF) { snprintf(error,errorSize,"PDF support is planned."); return false; }
    if (out->format == FOLIO_FORMAT_UNKNOWN) { snprintf(error,errorSize,"Use TXT, HTML, EPUB or PDF."); return false; }
    size_t n; char *raw = read_file(path, &n);
    if (!raw) { snprintf(error,errorSize,"Cannot read file (or it is larger than 4 MiB)."); return false; }
    char *decoded=folio_decode_bytes(raw,n,&n);free(raw);raw=decoded;
    if(!raw){snprintf(error,errorSize,"Unsupported or invalid text encoding.");return false;}
    if (out->format == FOLIO_FORMAT_HTML) { out->text = folio_markup_text(raw,n,&out->length); free(raw); }
    else { out->text = raw; out->length = n; }
    if (!out->text) { snprintf(error,errorSize,"Out of memory."); return false; }
    out->chapters[0].offset = 0; snprintf(out->chapters[0].title,sizeof(out->chapters[0].title),"Start"); out->chapterCount = 1;
    /* Plain-text convention: a short line beginning with # is a chapter heading. */
    for (size_t i=0; i<out->length && out->chapterCount<FOLIO_MAX_CHAPTERS;) {
        size_t start=i; while(i<out->length && out->text[i]!='\n') ++i; size_t len=i-start;
        if (len>2 && len<70 && out->text[start]=='#') { FolioChapter *c=&out->chapters[out->chapterCount++]; c->offset=start; snprintf(c->title,sizeof(c->title),"%.*s",(int)(len-1),out->text+start+1); }
        ++i;
    }
    return true;
}

void folio_free_document(FolioDocument *doc) { free(doc->text); memset(doc,0,sizeof(*doc)); }
