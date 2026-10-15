#include "settings_job_mock.h"
#include "settings_job.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
struct MockThread {pthread_t id;atomic_bool done;void (*fn)(void*);void *arg;};
static _Thread_local Thread current;
static _Thread_local int session;
static _Thread_local bool nonblocking;
static atomic_bool stallWrite;
static unsigned char records[2][1024];static u32 lengths[2];
static int failWrite,failService,writeCount;
static void *launch(void *p){Thread t=p;current=t;t->fn(t->arg);atomic_store(&t->done,true);return NULL;}
Thread threadCreate(void (*fn)(void*),void *arg,size_t stack,int priority,int core,bool detached){
 (void)stack;(void)priority;(void)core;assert(!detached);
 Thread t=calloc(1,sizeof(*t));t->fn=fn;t->arg=arg;assert(!pthread_create(&t->id,NULL,launch,t));return t;
}
Thread threadGetCurrent(void){return current;}
Result threadJoin(Thread t,u64 timeout){assert(timeout==0);if(!atomic_load(&t->done))return -1;return pthread_join(t->id,NULL);}
void threadFree(Thread t){free(t);}
void threadDetach(Thread t){pthread_detach(t->id);}
Result srvIsServiceRegistered(bool *v,const char *name){assert(!strcmp(name,"fs:USER"));*v=true;return 0;}
void srvSetBlockingPolicy(bool value){nonblocking=value;}
Result srvGetServiceHandleDirect(Handle *h,const char *name){assert(nonblocking&&!strcmp(name,"fs:USER"));if(failService)return -21;*h=77;return 0;}
Result FSUSER_Initialize(Handle h){assert(h==77);return 0;}
void fsUseSession(Handle h){session=h;}
void fsEndUseSession(void){session=0;}
Result svcCloseHandle(Handle h){assert(h==77);return 0;}
FS_Path fsMakePath(int type,const void *path){(void)type;return (FS_Path){path};}
Result FSUSER_OpenFileDirectly(Handle *h,int archive,FS_Path root,FS_Path path,u32 flags,u32 attrs){
 (void)root;(void)attrs;assert(current&&session==77&&archive==ARCHIVE_SDMC);
 int slot=!strcmp(path.data,"/Folio/preferences0.cfg")?0:!strcmp(path.data,"/Folio/preferences1.cfg")?1:-1;
 assert(slot>=0);if(flags==FS_OPEN_READ&&!lengths[slot])return -2;*h=slot+1;return 0;
}
Result FSFILE_Read(Handle h,u32 *bytes,u64 offset,void *data,u32 n){assert(offset==0);*bytes=lengths[h-1]<n?lengths[h-1]:n;memcpy(data,records[h-1],*bytes);return 0;}
Result FSFILE_Write(Handle h,u32 *bytes,u64 offset,const void *data,u32 n,u32 flags){
 assert(offset==0&&flags==FS_WRITE_FLUSH&&n<=1024);
 while(atomic_load(&stallWrite))usleep(1000);
 if(failWrite)return -12;
 writeCount++;memcpy(records[h-1],data,n);lengths[h-1]=n;*bytes=n;return 0;
}
Result FSFILE_Close(Handle h){assert(h==1||h==2);return 0;}
static int await_job(FolioSettings *out){
 for(int i=0;i<2000;i++){int result=settings_job_poll(out);if(result)return result;usleep(1000);}
 assert(!"job did not complete");return 0;
}
int main(void){
 FolioSettings s={.language=1},out={0};strcpy(s.folder,"sdmc:/Books/");
 atomic_store(&stallWrite,true);assert(settings_job_start(&s));
 for(int i=0;i<2000&&settings_job_stage()!=7;i++)usleep(1000);
 assert(settings_job_stage()==7);assert(settings_job_busy());assert(!settings_job_poll(&out));
 assert(!settings_job_start(&s));assert(writeCount==0);
 /* Polling remains immediate while an actual worker is held inside Write. */
 for(int i=0;i<10000;i++)assert(settings_job_poll(&out)==0);
 atomic_store(&stallWrite,false);assert(await_job(&out)==1);assert(out.generation==1&&out.language==1);
 assert(writeCount==1&&!settings_job_busy());
 s=out;s.language=0;failWrite=1;assert(settings_job_start(&s));assert(await_job(&out)==-1);assert(settings_job_error()==-12);
 assert(writeCount==1);
 failWrite=0;assert(settings_job_start(&s));assert(await_job(&out)==1);assert(out.generation==2&&out.language==0);
 failService=1;assert(settings_job_start(&s));assert(await_job(&out)==-1);assert(settings_job_error()==-21);
 settings_job_shutdown();puts("PASS: asynchronous native settings, stalled write, nonblocking polling, immutable request, write/service errors and recovery");return 0;
}
