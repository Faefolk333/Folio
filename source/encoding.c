#include "encoding.h"
#include "unicode_tables.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define LIMIT (4u*1024u*1024u)
static size_t put(char *s,unsigned c){
 if(c<128){s[0]=c;return 1;}if(c<2048){s[0]=0xc0|(c>>6);s[1]=0x80|(c&63);return 2;}
 if(c<65536){s[0]=0xe0|(c>>12);s[1]=0x80|((c>>6)&63);s[2]=0x80|(c&63);return 3;}
 s[0]=0xf0|(c>>18);s[1]=0x80|((c>>12)&63);s[2]=0x80|((c>>6)&63);s[3]=0x80|(c&63);return 4;
}
static unsigned get(const unsigned char *s,size_t n,size_t *p){
 if(*p>=n)return 0xffffffffu;
 unsigned c=s[(*p)++],v;int count;
 if(c<128)return c;
 if(c>=0xc2&&c<=0xdf){v=c&31;count=1;}else if(c>=0xe0&&c<=0xef){v=c&15;count=2;}else if(c>=0xf0&&c<=0xf4){v=c&7;count=3;}else return 0xffffffffu;
 if(*p+count>n)return 0xffffffffu;
 for(int i=0;i<count;i++){unsigned b=s[(*p)++];if((b&0xc0)!=0x80)return 0xffffffffu;v=(v<<6)|(b&63);}
 if(v>0x10ffff||(v>=0xd800&&v<=0xdfff)||(count==1&&v<128)||(count==2&&v<2048)||(count==3&&v<65536))return 0xffffffffu;
 return v;
}
void folio_compose_utf8(char *s,size_t *n){
 size_t p=0,w=0,last=0;unsigned previous=0;
 while(p<*n){unsigned cp=get((unsigned char*)s,*n,&p);if(cp==0xffffffffu)cp=0xfffd;
  unsigned combined=0;
  if(cp>=0x300&&cp<=0x36f)
   for(size_t i=0;i<sizeof(compositions)/sizeof(compositions[0]);i++)if(compositions[i].base==previous&&compositions[i].mark==cp){combined=compositions[i].result;break;}
  if(combined){w=last;w+=put(s+w,combined);previous=combined;}
  else{last=w;w+=put(s+w,cp);previous=cp;}
 }
 s[w]=0;*n=w;
}
unsigned folio_named_entity(const char *s,size_t n){
 for(size_t i=0;i<sizeof(named_entities)/sizeof(named_entities[0]);i++)if(strlen(named_entities[i].name)==n&&!memcmp(s,named_entities[i].name,n))return named_entities[i].code;
 return 0;
}
/* Only inspect actual XML declarations / HTML meta tags, never book prose. */
static void declaration(const char *s,size_t n,char name[40]){
 char head[1025];size_t take=n<1024?n:1024;
 for(size_t i=0;i<take;i++)head[i]=tolower((unsigned char)s[i]);
 head[take]=0;name[0]=0;
 char *p=head;
 while((p=strchr(p,'<'))){char *end=strchr(p,'>');if(!end)break;
  if(!strncmp(p,"<?xml",5)||!strncmp(p,"<meta",5)){
   for(char *q=p;q<end;q++)if(!strncmp(q,"encoding",8)||!strncmp(q,"charset",7)){
    q+=!strncmp(q,"encoding",8)?8:7;while(q<end&&isspace((unsigned char)*q))q++;
    if(q==end||*q!='=')break;
    q++;while(q<end&&(isspace((unsigned char)*q)||*q=='\''||*q=='"'))q++;
    size_t w=0;while(q<end&&w<39&&(isalnum((unsigned char)*q)||*q=='-'||*q=='_'))name[w++]=*q++;name[w]=0;return;
   }
  }p=end+1;
 }
}
char *folio_decode_bytes(const char *input,size_t n,size_t *outN){
 if(n>LIMIT)return NULL;
 const unsigned char *s=(const unsigned char*)input;size_t p=0,w=0;int mode=0;const unsigned *map=NULL;char declared[40];
 if(n>=2&&s[0]==0xff&&s[1]==0xfe){mode=1;p=2;}
 else if(n>=2&&s[0]==0xfe&&s[1]==0xff){mode=2;p=2;}
 else if(n>=4&&s[0]=='<'&&s[1]==0&&s[2]=='?'&&s[3]==0)mode=1;
 else if(n>=4&&s[0]==0&&s[1]=='<'&&s[2]==0&&s[3]=='?')mode=2;
 else{
  declaration(input,n,declared);
  if(!strcmp(declared,"windows-1250")||!strcmp(declared,"cp1250"))map=map_cp1250;
  else if(!strcmp(declared,"iso-8859-2")||!strcmp(declared,"iso8859-2"))map=map_iso8859_2;
  else if(!strcmp(declared,"windows-1252")||!strcmp(declared,"cp1252"))map=map_cp1252;
  else if(declared[0]&&strcmp(declared,"utf-8")&&strcmp(declared,"utf8")&&strcmp(declared,"us-ascii"))return NULL;
  if(!map){size_t check=0;
   int valid=1;while(check<n){unsigned cp=get(s,n,&check);if(cp==0xffffffffu||cp==0){valid=0;break;}}
   if(!valid){if(declared[0])return NULL;map=map_cp1250;}
  }
  if(!map&&n>=3&&!memcmp(s,"\xef\xbb\xbf",3))p=3;
 }
 char *out=malloc(n*3+4);if(!out)return NULL;
 while(p<n){unsigned cp;
  if(mode){if(p+2>n)goto fail;cp=mode==1?s[p]|s[p+1]<<8:s[p]<<8|s[p+1];p+=2;
   if(cp>=0xd800&&cp<=0xdbff){if(p+2>n)goto fail;unsigned low=mode==1?s[p]|s[p+1]<<8:s[p]<<8|s[p+1];p+=2;if(low<0xdc00||low>0xdfff)goto fail;cp=0x10000+((cp-0xd800)<<10)+(low-0xdc00);}
   else if(cp>=0xdc00&&cp<=0xdfff)goto fail;
  }else if(map){cp=s[p++];if(cp>=128)cp=map[cp-128];}else cp=get(s,n,&p);
  if(cp==0||cp==0xffffffffu)goto fail;
  w+=put(out+w,cp);if(w>LIMIT)goto fail;
 }
 out[w]=0;folio_compose_utf8(out,&w);*outN=w;return out;
fail:free(out);return NULL;
}
