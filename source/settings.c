#include "settings.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#ifdef __3DS__
#include "settings_job.h"
#endif
/* Alternating, checksummed records: a short/torn write preserves the other slot. */
typedef struct {
 uint32_t magic,version,generation,language;
 char folder[FOLIO_PATH_MAX];
 uint32_t checksum;
} DiskSettings;
static uint32_t checksum(const DiskSettings *d){
 const unsigned char *p=(const unsigned char*)d;uint32_t h=2166136261u;
 for(size_t i=0;i<offsetof(DiskSettings,checksum);i++){h^=p[i];h*=16777619u;}return h;
}
bool settings_folder_valid(const char *p){
 size_t n=strlen(p);if(!n)return true;
 if(n>=FOLIO_PATH_MAX||strncmp(p,"sdmc:/",6)||p[n-1]!='/')return false;
 for(size_t i=0;i<n;i++)if((unsigned char)p[i]<32||p[i]=='\\')return false;
 if(strstr(p,"/../")||strstr(p,"/./")||strchr(p+6,':'))return false;
 return true;
}
static bool read_slot(const char *root,int slot,DiskSettings *d){
 char path[768];snprintf(path,sizeof(path),"%s/preferences%d.cfg",root,slot);
 bool ok;
#ifdef __3DS__
 if(settings_io_active())ok=settings_io_read(path,d,sizeof(*d));
 else
#endif
 {
  FILE *f=fopen(path,"rb");if(!f)return false;
  ok=fread(d,1,sizeof(*d),f)==sizeof(*d);fclose(f);
 }
 return ok&&d->magic==0x464f4c34&&d->version==1&&d->language<2&&
  memchr(d->folder,0,sizeof(d->folder))&&settings_folder_valid(d->folder)&&d->checksum==checksum(d);
}
bool settings_load(const char *root,FolioSettings *out){
 memset(out,0,sizeof(*out));DiskSettings a,b;bool va=read_slot(root,0,&a),vb=read_slot(root,1,&b);
 if(!va&&!vb)return false;
 DiskSettings *d=va&&(!vb||(int32_t)(a.generation-b.generation)>0)?&a:&b;
 out->generation=d->generation;out->language=d->language;strcpy(out->folder,d->folder);return true;
}
bool settings_save(const char *root,FolioSettings *s){
 if(s->language<0||s->language>1||!settings_folder_valid(s->folder))return false;
 FolioSettings latest;
 if(settings_load(root,&latest)&&(int32_t)(latest.generation-s->generation)>0)s->generation=latest.generation;
 DiskSettings d={0};d.magic=0x464f4c34;d.version=1;d.generation=s->generation+1;d.language=s->language;
 strcpy(d.folder,s->folder);d.checksum=checksum(&d);
 char path[768];snprintf(path,sizeof(path),"%s/preferences%u.cfg",root,(unsigned)(d.generation%2));
 bool ok;
#ifdef __3DS__
 if(settings_io_active())ok=settings_io_write(path,&d,sizeof(d));
 else return false; /* Never write preferences synchronously on the UI thread. */
#else
 FILE *f=fopen(path,"wb");if(!f)return false;
 ok=fwrite(&d,1,sizeof(d),f)==sizeof(d);if(fclose(f)!=0)ok=false;
#endif
 if(ok)s->generation=d.generation;
 return ok;
}
