#include "folio.h"
#include <stdio.h>
int main(int argc,char **argv) {
 if(argc!=2)return 2;
 FolioDocument doc;char error[160];
 if(!folio_load_document(argv[1],&doc,error,sizeof(error))){fprintf(stderr,"%s\n",error);return 1;}
 printf("TITLE:%s\nCHAPTERS:%d\n",doc.title,doc.chapterCount);
 for(int i=0;i<doc.chapterCount;i++)printf("CHAPTER:%zu:%s\n",doc.chapters[i].offset,doc.chapters[i].title);
 puts(doc.text);folio_free_document(&doc);return 0;
}
