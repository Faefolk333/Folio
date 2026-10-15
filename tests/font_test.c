#include "preview_platform.h"
#include "reader.h"
#include <assert.h>
int main(void){
 const char *files[]={"sans","serif","inter","ebgaramond","librebaskerville"};
 const char *letters="ěščřžýáíéúůďťňĚŠČŘŽÝÁÍÉÚŮĎŤŇ";
 for(int i=0;i<5;i++){
  char path[128];snprintf(path,sizeof(path),"assets/fonts/%s.bcfnt",files[i]);C2D_Font f=preview_font(path,i);
  size_t p=0;while(p<strlen(letters)){unsigned cp=reader_codepoint(letters,strlen(letters),&p);int index=C2D_FontGlyphIndexFromCodePoint(f,cp);assert(index!=read16(f->data+30));assert(C2D_FontGetCharWidthInfo(f,index)->charWidth>0);}
  free(f->data);free(f);
 }
 puts("PASS: all Czech letters present in all five embedded font atlases");return 0;
}
