#define FOLIO_HOST_PREVIEW
#include "../source/main.c"
#include <assert.h>
int main(void){
 Entry sample[]={
  {.name="Alpha.epub",.lastRead=100},
  {.name="Unread B.txt"},
  {.name="Zeta.epub",.lastRead=500},
  {.name="Unread A.txt"},
  {.name="Folder/",.directory=true,.lastRead=999},
  {.name="Beta.html",.lastRead=100}
 };
 qsort(sample,6,sizeof(sample[0]),compare_entries);
 assert(sample[0].directory);
 assert(!strcmp(sample[1].name,"Zeta.epub"));
 assert(!strcmp(sample[2].name,"Alpha.epub"));
 assert(!strcmp(sample[3].name,"Beta.html"));
 assert(!strcmp(sample[4].name,"Unread A.txt"));
 assert(!strcmp(sample[5].name,"Unread B.txt"));
 sample[5].lastRead=600;qsort(sample,6,sizeof(sample[0]),compare_entries);
 assert(!strcmp(sample[1].name,"Unread B.txt"));
 puts("PASS: recent-first library, unread and equal-date alphabetical order, directories, changed recency");
 return 0;
}
