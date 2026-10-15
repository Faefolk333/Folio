#include "settings.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){
 assert(argc==2);FolioSettings s;
 assert(!settings_load(argv[1],&s));assert(s.language==0&&!s.folder[0]);
 assert(settings_folder_valid("sdmc:/Books/"));assert(settings_folder_valid("sdmc:/České knihy/"));
 assert(!settings_folder_valid("sdmc:/Books/../"));assert(!settings_folder_valid("/Books/"));
 s.language=1;strcpy(s.folder,"sdmc:/České knihy/");assert(settings_save(argv[1],&s));
 FolioSettings read;assert(settings_load(argv[1],&read));assert(read.language==1&&!strcmp(s.folder,read.folder));
 s.language=0;assert(settings_save(argv[1],&s));assert(settings_load(argv[1],&read));assert(read.language==0);
 char path[768];snprintf(path,sizeof(path),"%s/preferences%u.cfg",argv[1],(unsigned)(s.generation%2));
 FILE *f=fopen(path,"wb");assert(f);fputs("incomplete",f);fclose(f);
 assert(settings_load(argv[1],&read));assert(read.language==1); /* Previous slot survives. */
 read.language=9;assert(!settings_save(argv[1],&read));
 puts("PASS: English defaults, Czech path, persisted language/folder, torn-write recovery, validation");return 0;
}
