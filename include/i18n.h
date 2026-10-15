#pragma once
enum {LANG_EN,LANG_CS,LANG_COUNT};
void i18n_set(int language);
const char *tr(const char *english,const char *czech);
const char *i18n_error(const char *english);
