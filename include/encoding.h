#pragma once
#include <stddef.h>
char *folio_decode_bytes(const char *input,size_t length,size_t *outputLength);
void folio_compose_utf8(char *text,size_t *length);
unsigned folio_named_entity(const char *name,size_t length);
