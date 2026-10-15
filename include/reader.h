#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define READER_FONT_COUNT 6

typedef struct {
 int theme, font, size, leading, tracking, brightness, strength, sleepSeconds;
 bool bionic, focus;
} ReaderOptions;
typedef struct { size_t start,end; float width; } ReaderLine;
typedef struct {
 ReaderLine *lines;
 size_t count;
 int rows;
 float height;
} ReaderLayout;
typedef float (*ReaderMeasure)(uint32_t codepoint,void *context);
ReaderOptions reader_defaults(void);
uint32_t reader_codepoint(const char *text,size_t length,size_t *offset);
bool reader_layout(ReaderLayout *out,const char *text,size_t length,const ReaderOptions *options,ReaderMeasure measure,void *context);
void reader_layout_free(ReaderLayout *layout);
size_t reader_find_line(const ReaderLayout *layout,size_t offset);
int reader_visible(const ReaderLayout *layout,size_t first);
bool reader_bookmark_visible(const ReaderLayout *layout,size_t first,size_t bookmark);
int reader_focus_move(int current,int delta,int visible);
int reader_bold_prefix(int characters,int strength);
bool reader_word_char(uint32_t codepoint);
