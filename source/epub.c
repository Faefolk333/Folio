/* Bounded, read-only EPUB 2/3 text reader. ZIP entries are never extracted to SD. */
#include "epub.h"
#include "encoding.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>

#define LIMIT (4u*1024u*1024u)
#define MAX_FILES 4096
typedef struct { char name[512];
 uint32_t offset, packed, size, crc;
 unsigned method, flags;
 } Entry;

typedef struct { FILE *file;
 Entry *entries;
 unsigned count;
 long length;
 } Zip;

static unsigned u16(const unsigned char *p) { return p[0]|(unsigned)p[1]<<8;
 }
static uint32_t u32(const unsigned char *p) { return u16(p)|(uint32_t)u16(p+2)<<16;
 }
static void zip_close(Zip *z) { if(z->file)fclose(z->file);
free(z->entries);
 }
static int zip_open(Zip *z,const char *path) {
 memset(z,0,sizeof(*z));
 z->file=fopen(path,"rb");

 if(!z->file)return 0;

 if(fseek(z->file,0,SEEK_END))return 0;

 z->length=ftell(z->file);

 if(z->length<22)return 0;

 size_t n=z->length>65557?65557:(size_t)z->length;

 unsigned char *tail=malloc(n);

 if(!tail)return 0;

 if(fseek(z->file,z->length-(long)n,SEEK_SET)||fread(tail,1,n,z->file)!=n){free(tail);
return 0;
}
 long e=(long)n-22;

 while(e>=0 && (u32(tail+e)!=0x06054b50 || (size_t)e+22+u16(tail+e+20)!=n))--e;

 if(e<0){free(tail);
return 0;
}
 unsigned count=u16(tail+e+10);
uint32_t off=u32(tail+e+16),sz=u32(tail+e+12);

 int valid=!u16(tail+e+4)&&!u16(tail+e+6)&&u16(tail+e+8)==count&&count>0&&count<=MAX_FILES&&(uint64_t)off+sz<=(uint64_t)(z->length-(long)n+e);

 free(tail);

 if(!valid)return 0;

 z->entries=calloc(count,sizeof(Entry));

 if(!z->entries)return 0;
z->count=count;

 if(fseek(z->file,off,SEEK_SET))return 0;

 for(unsigned i=0;
i<count;
++i){
  unsigned char h[46];

 if(fread(h,1,46,z->file)!=46||u32(h)!=0x02014b50)return 0;

  unsigned len=u16(h+28),extra=u16(h+30),comment=u16(h+32);
Entry *a=&z->entries[i];

  if(!len||len>=sizeof(a->name)||u16(h+34))return 0;

  if(fread(a->name,1,len,z->file)!=len||memchr(a->name,0,len))return 0;

  a->flags=u16(h+8);
a->method=u16(h+10);
a->crc=u32(h+16);
a->packed=u32(h+20);
a->size=u32(h+24);
a->offset=u32(h+42);

  if(fseek(z->file,extra+comment,SEEK_CUR)||(uint64_t)ftell(z->file)>(uint64_t)off+sz)return 0;

 }
 return 1;

}
static char *zip_read(Zip *z,const char *name,size_t *size){
 Entry *e=NULL;
for(unsigned i=0;
i<z->count;
++i)if(!strcmp(z->entries[i].name,name)){e=&z->entries[i];
break;
}
 if(!e||e->size>LIMIT||e->packed>LIMIT||(e->flags&1)||(e->method!=0&&e->method!=8))return NULL;

 unsigned char h[30];

 if((uint64_t)e->offset+30>(uint64_t)z->length||fseek(z->file,e->offset,SEEK_SET)||fread(h,1,30,z->file)!=30||u32(h)!=0x04034b50)return NULL;

 uint64_t start=(uint64_t)e->offset+30+u16(h+26)+u16(h+28);

 if(start+e->packed>(uint64_t)z->length||fseek(z->file,(long)start,SEEK_SET))return NULL;

 char *out=malloc(e->size+1);
unsigned char *data=malloc(e->packed+1);

 if(!out||!data){free(out);
free(data);
return NULL;
}
 int ok=fread(data,1,e->packed,z->file)==e->packed;

 if(ok&&e->method==0){ok=e->packed==e->size;

 if(ok)memcpy(out,data,e->size);
}
 else if(ok){z_stream stream={0};
stream.next_in=data;
stream.avail_in=e->packed;
stream.next_out=(Bytef*)out;
stream.avail_out=e->size+1;

  ok=inflateInit2(&stream,-MAX_WBITS)==Z_OK;

  if(ok){ok=inflate(&stream,Z_FINISH)==Z_STREAM_END&&stream.total_out==e->size&&stream.total_in==e->packed;
inflateEnd(&stream);
}
 }
 free(data);

 if(!ok||crc32(0,(Bytef*)out,e->size)!=e->crc){free(out);
return NULL;
}
 out[e->size]=0;
*size=e->size;
return out;

}

static char *zip_text(Zip *z,const char *name,size_t *size){
 char *raw=zip_read(z,name,size);if(!raw)return NULL;
 char *decoded=folio_decode_bytes(raw,*size,size);free(raw);return decoded;
}
/* XML token scanner: comments, quoted '>', namespace prefixes and attributes. */
typedef struct { const char *attrs,*end,*after;
 char name[64];
 int closing;
 } Tag;

static int next_tag(const char **cursor,Tag *t){
 const char *p=*cursor;

 while((p=strchr(p,'<'))){
  if(!strncmp(p,"<!--",4)){p=strstr(p+4,"-->");

 if(!p)return 0;
p+=3;
continue;
}
  const char *q=p+1;
char quote=0;

  for(;
*q;
++q){if(quote){if(*q==quote)quote=0;
}else if(*q=='"'||*q=='\'')quote=*q;
else if(*q=='>')break;
}
  if(!*q)return 0;

  *cursor=q+1;

  if(p[1]=='!'||p[1]=='?'){p=q+1;
continue;
}
  memset(t,0,sizeof(*t));
p++;

 if(*p=='/'){t->closing=1;
p++;
}
  const char *name=p;
while(p<q&&!isspace((unsigned char)*p)&&*p!='/'){p++;
}
  const char *local=name;
for(const char *x=name;
x<p;
++x)if(*x==':')local=x+1;

  size_t n=p-local;

 if(n>=sizeof(t->name))n=sizeof(t->name)-1;
memcpy(t->name,local,n);

  t->attrs=p;
t->end=q;
t->after=q+1;
return 1;

 }
 return 0;

}
static unsigned entity(const char *s,size_t n){
 if(n==3&&!memcmp(s,"amp",3))return '&';

 if(n==2&&!memcmp(s,"lt",2))return '<';

 if(n==2&&!memcmp(s,"gt",2))return '>';

 if(n==4&&!memcmp(s,"quot",4))return '"';

 if(n==4&&!memcmp(s,"apos",4))return 39;

 if(n==4&&!memcmp(s,"nbsp",4))return 32;

 if(n==5&&!memcmp(s,"mdash",5))return 0x2014;

 if(n==5&&!memcmp(s,"ndash",5))return 0x2013;

 if(n>1&&s[0]=='#'){unsigned v=0,base=10;
size_t i=1;

 if(s[i]=='x'||s[i]=='X'){base=16;
i++;
}if(i==n)return 0;

  for(;
i<n;
++i){unsigned d=isdigit((unsigned char)s[i])?s[i]-'0':tolower((unsigned char)s[i])-'a'+10;

 if(d>=base||v>0x10ffff/base)return 0;
v=v*base+d;
}
  if(v>0&&v<=0x10ffff&&!(v>=0xd800&&v<=0xdfff))return v;

 }return folio_named_entity(s,n);

}
static size_t utf8(char *p,unsigned c){if(c<128){p[0]=c;
return 1;
}if(c<2048){p[0]=0xc0|(c>>6);
p[1]=0x80|(c&63);
return 2;
}if(c<65536){p[0]=0xe0|(c>>12);
p[1]=0x80|((c>>6)&63);
p[2]=0x80|(c&63);
return 3;
}p[0]=0xf0|(c>>18);
p[1]=0x80|((c>>12)&63);
p[2]=0x80|((c>>6)&63);
p[3]=0x80|(c&63);
return 4;
}
static void decode(char *out,const char *p,size_t n){size_t w=0;
for(size_t i=0;
i<n;
++i){if(p[i]=='&'){const char *e=memchr(p+i,';',n-i);

 if(e){unsigned c=entity(p+i+1,e-p-i-1);

 if(c){w+=utf8(out+w,c);
i=e-p;
continue;
}}}out[w++]=p[i];
}out[w]=0;
}
static int attr(Tag *t,const char *key,char *out,size_t cap){
 const char *p=t->attrs;
while(p<t->end){
  while(p<t->end&&(isspace((unsigned char)*p)||*p=='/')){p++;
}
  const char *s=p;
while(p<t->end&&!isspace((unsigned char)*p)&&*p!='='){p++;
}size_t n=p-s;

  while(p<t->end&&isspace((unsigned char)*p)){p++;
}
 if(p==t->end||*p++!='=')return 0;

  while(p<t->end&&isspace((unsigned char)*p)){p++;
}
 if(p==t->end||(*p!='"'&&*p!=39))return 0;

  char q=*p++;
const char *v=p;
while(p<t->end&&*p!=q){p++;
}
 if(p==t->end)return 0;

  if(strlen(key)==n&&!memcmp(s,key,n)){if((size_t)(p-v)>=cap)return 0;
decode(out,v,p-v);
return 1;
}p++;

 }return 0;

}
char *folio_markup_text(const char *input,size_t length,size_t *outputLength){
 char *out=malloc(length+2);

 if(!out)return NULL;
size_t w=0;
const char *p=input;
char hidden[64]={0};

 while(*p){
  if(*p=='<'){const char *start=p;
Tag t;

 if(!next_tag(&p,&t))break;

   if(hidden[0]){if(t.closing&&!strcmp(t.name,hidden))hidden[0]=0;
continue;
}
   if(!t.closing&&(!strcmp(t.name,"head")||!strcmp(t.name,"script")||!strcmp(t.name,"style"))){strcpy(hidden,t.name);
continue;
}
   if(!strcmp(t.name,"p")||!strcmp(t.name,"div")||!strcmp(t.name,"br")||!strcmp(t.name,"li")||!strcmp(t.name,"section")||(t.name[0]=='h'&&t.name[1]>='1'&&t.name[1]<='6'&&!t.name[2])){
    if(w&&out[w-1]!='\n')out[w++]='\n';

   }if(p<=start)break;
continue;

  }
  if(hidden[0]){p++;
continue;
}
  if(*p=='&'){const char *e=strchr(p,';');

 if(e&&e-p<16){unsigned c=entity(p+1,e-p-1);

 if(c){w+=utf8(out+w,c);
p=e+1;
continue;
}}}
  if(isspace((unsigned char)*p)){if(w&&out[w-1]!=' '&&out[w-1]!='\n')out[w++]=' ';
p++;
continue;
}
  out[w++]=*p++;

 }
 while(w&&isspace((unsigned char)out[w-1])){w--;
}out[w]=0;
folio_compose_utf8(out,&w);
*outputLength=w;
return out;

}
static int resolve(const char *opf,const char *href,char *out){
 char joined[1024],decoded[1024];
const char *slash=strrchr(opf,'/');
size_t base=slash?(size_t)(slash-opf+1):0;

 if(base+strlen(href)>=sizeof(joined)||strchr(href,':')||href[0]=='/')return 0;

 memcpy(joined,opf,base);
strcpy(joined+base,href);
size_t w=0;

 for(size_t i=0;
joined[i]&&joined[i]!='#';
++i){if(joined[i]=='%'){if(!isxdigit((unsigned char)joined[i+1])||!joined[i+2]||!isxdigit((unsigned char)joined[i+2]))return 0;
char h[3]={joined[i+1],joined[i+2],0};
char c=(char)strtoul(h,NULL,16);

 if(!c)return 0;
decoded[w++]=c;
i+=2;
}else decoded[w++]=joined[i];
}decoded[w]=0;

 out[0]=0;
char *p=decoded;

 while(*p){char *e=strchr(p,'/');

 if(e)*e=0;

  if(!strcmp(p,"..")){char *s=strrchr(out,'/');

 if(s)*s=0;
else if(out[0])out[0]=0;
else return 0;
}
  else if(strcmp(p,".")&&*p){if(strlen(out)+strlen(p)+2>512)return 0;

 if(out[0])strcat(out,"/");
strcat(out,p);
}
  if(!e){break;
}p=e+1;

 }return out[0]!=0;

}
bool folio_load_epub(const char *path,FolioDocument *doc,char *error,size_t cap){
 Zip z={0};
char *container=NULL,*opf=NULL,*chapter=NULL,*text=NULL;
size_t n;
const char *reason="Invalid or unsupported EPUB archive.";

 if(!zip_open(&z,path))goto fail;

 container=zip_text(&z,"META-INF/container.xml",&n);

 if(!container)goto fail;

 char package[512]={0};
const char *p=container;
Tag t;

 while(next_tag(&p,&t))if(!t.closing&&!strcmp(t.name,"rootfile")&&attr(&t,"full-path",package,sizeof(package)))break;

 if(!package[0]){goto fail;
}
 opf=zip_text(&z,package,&n);

 if(!opf)goto fail;

 /* Refuse declared encryption rather than displaying corrupted content. */
 for(unsigned i=0;
i<z.count;
i++)if(!strcmp(z.entries[i].name,"META-INF/encryption.xml")){reason="Encrypted/DRM EPUB is not supported.";
goto fail;
}
 p=opf;
while(next_tag(&p,&t))if(!t.closing&&!strcmp(t.name,"title")){const char *end=strchr(t.after,'<');

 if(end&&(size_t)(end-t.after)<sizeof(doc->title))decode(doc->title,t.after,end-t.after);
break;
}
 doc->text=calloc(1,1);

 if(!doc->text){reason="Out of memory.";
goto fail;
}
 p=opf;
int inSpine=0;

 while(next_tag(&p,&t)){
  if(!strcmp(t.name,"spine")){inSpine=!t.closing;
continue;
}
  if(!inSpine||t.closing||strcmp(t.name,"itemref"))continue;

  char id[256],linear[32];

 if(attr(&t,"linear",linear,sizeof(linear))&&!strcmp(linear,"no"))continue;

  if(!attr(&t,"idref",id,sizeof(id)))goto fail;

  const char *m=opf;
Tag item;
char href[512]={0},type[96]={0};
int manifest=0;

  while(next_tag(&m,&item)){if(!strcmp(item.name,"manifest")){manifest=!item.closing;
continue;
}char found[256];

   if(manifest&&!item.closing&&!strcmp(item.name,"item")&&attr(&item,"id",found,sizeof(found))&&!strcmp(found,id)){attr(&item,"href",href,sizeof(href));
attr(&item,"media-type",type,sizeof(type));
break;
}}
  if(strcmp(type,"application/xhtml+xml")&&strcmp(type,"text/html")){reason="Unsupported EPUB chapter format.";
goto fail;
}
  char name[512];

 if(!href[0]||!resolve(package,href,name))goto fail;

  chapter=zip_text(&z,name,&n);

 if(!chapter){reason="Cannot decode chapter: encoding, corrupt data or 4 MiB limit.";
goto fail;
}
  text=folio_markup_text(chapter,n,&n);
free(chapter);
chapter=NULL;

 if(!text){reason="Out of memory.";
goto fail;
}
  if(n){
   if(doc->length+n+2>LIMIT){reason="EPUB text exceeds 4 MiB.";
goto fail;
}
   char *grown=realloc(doc->text,doc->length+n+3);

 if(!grown){reason="Out of memory.";
goto fail;
}doc->text=grown;

   if(doc->chapterCount<FOLIO_MAX_CHAPTERS){FolioChapter *c=&doc->chapters[doc->chapterCount++];
c->offset=doc->length;
size_t k=strcspn(text,"\n");

 if(k>=sizeof(c->title))k=sizeof(c->title)-1;
while(k&&((unsigned char)text[k]&0xc0)==0x80)--k;
memcpy(c->title,text,k);
c->title[k]=0;
}
   memcpy(doc->text+doc->length,text,n);
doc->length+=n;
doc->text[doc->length++]='\n';
doc->text[doc->length++]='\n';
doc->text[doc->length]=0;

  }free(text);
text=NULL;

 }
 if(!doc->length){reason="EPUB contains no readable text.";
goto fail;
}
 free(container);
free(opf);
zip_close(&z);
return true;

fail:
 free(container);
free(opf);
free(chapter);
free(text);
free(doc->text);
doc->text=NULL;
doc->length=0;
doc->chapterCount=0;
zip_close(&z);
snprintf(error,cap,"%s",reason);
return false;

}
