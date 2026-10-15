#include "reader.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

ReaderOptions reader_defaults(void) {
 return (ReaderOptions){.theme=1,.font=3,.size=17,.leading=147,.tracking=0,
  .brightness=3,.strength=1,.sleepSeconds=20};
}
uint32_t reader_codepoint(const char *s,size_t n,size_t *at) {
 if(*at>=n)return 0;
 unsigned c=(unsigned char)s[(*at)++];
 if(c<128)return c;
 unsigned value;int extra;
 if(c>=0xc2&&c<=0xdf){value=c&31;extra=1;}
 else if(c>=0xe0&&c<=0xef){value=c&15;extra=2;}
 else if(c>=0xf0&&c<=0xf4){value=c&7;extra=3;}
 else return 0xfffd;
 if(*at+(size_t)extra>n)return 0xfffd;
 for(int i=0;i<extra;i++){
  unsigned b=(unsigned char)s[*at+i];
  if((b&0xc0)!=0x80)return 0xfffd;
  value=(value<<6)|(b&63);
 }
 *at+=extra;
 if(value>0x10ffff||(value>=0xd800&&value<=0xdfff)||
   (extra==1&&value<128)||(extra==2&&value<2048)||(extra==3&&value<65536))return 0xfffd;
 return value;
}
void reader_layout_free(ReaderLayout *l){free(l->lines);memset(l,0,sizeof(*l));}
bool reader_layout(ReaderLayout *out,const char *s,size_t n,const ReaderOptions *o,ReaderMeasure measure,void *ctx){
 ReaderLayout result={0};
 result.height=o->size*o->leading/100.0f;
 result.rows=(int)floorf(178.0f/result.height);
 if(result.rows<1)result.rows=1;
 size_t capacity=256,p=0;
 result.lines=malloc(capacity*sizeof(*result.lines));
 if(!result.lines)return false;
 while(p<n){
  size_t start=p,end=p,space=0;float width=0,spaceWidth=0;
  while(p<n){
   size_t before=p;uint32_t c=reader_codepoint(s,n,&p);
   if(c=='\r'){end=p;continue;}
   if(c=='\n'){end=before;break;}
   float advance=measure(c,ctx)+o->size*o->tracking/100.0f;
   if(advance<0.5f)advance=0.5f;
   if(width+advance>356.0f&&before>start){
    if(space>start){p=space;end=space;width=spaceWidth;}
    else{p=before;end=before;}
    break;
   }
   width+=advance;end=p;
   if(c==' '||c=='\t'){space=p;spaceWidth=width;}
  }
  if(result.count==capacity){capacity*=2;ReaderLine *grown=realloc(result.lines,capacity*sizeof(*grown));
   if(!grown){reader_layout_free(&result);return false;}result.lines=grown;}
  result.lines[result.count++]=(ReaderLine){start,end,width};
 }
 if(!result.count)result.lines[result.count++]=(ReaderLine){0,0,0};
 reader_layout_free(out);*out=result;return true;
}
size_t reader_find_line(const ReaderLayout *l,size_t p){
 if(!l->count)return 0;
 size_t low=0,high=l->count;
 while(low+1<high){size_t mid=(low+high)/2;if(l->lines[mid].start<=p)low=mid;else high=mid;}
 return low;
}
int reader_visible(const ReaderLayout *l,size_t first){
 if(first>=l->count)return 0;
 size_t remaining=l->count-first;return remaining<(size_t)l->rows?(int)remaining:l->rows;
}
int reader_focus_move(int current,int delta,int visible){
 if(visible<=0)return 0;
 int v=current+delta;return v<0?0:v>=visible?visible-1:v;
}
bool reader_word_char(uint32_t c){
 return (c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||
  (c>=0xc0&&c<=0x2af)||(c>=0x370&&c<=0x1fff)||(c>=0x2c00&&c<=0xd7ff);
}
bool reader_bookmark_visible(const ReaderLayout *l,size_t first,size_t bookmark){
 size_t line=reader_find_line(l,bookmark);
 return line>=first&&line<first+(size_t)reader_visible(l,first);
}
int reader_bold_prefix(int n,int strength){
 if(n<=0)return 0;
 if(strength==0)return n<=2?0:(int)fmaxf(1,roundf(n*.28f));
 if(strength==2)return n<=2?n:(int)fmaxf(2,ceilf(n*.52f));
 return n<=2?n:n<=4?2:n<=7?3:(int)fmaxf(3,roundf(n*.40f));
}
