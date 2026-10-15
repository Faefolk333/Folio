#include "shutdown_mock.h"
#include "shutdown_save.h"
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
struct MockThread{pthread_t id;atomic_bool done;void(*fn)(void*);void *arg;};
static int mode,detached,closed;static atomic_int writes;static char output[128];
static _Thread_local Handle session;
static double seconds(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static void *launch(void *p){Thread t=p;t->fn(t->arg);atomic_store(&t->done,true);return NULL;}
Thread threadCreate(void(*fn)(void*),void *arg,size_t stack,int priority,int core,bool detach){
 assert(stack>=16384&&priority==0x31&&core==0&&!detach);
 Thread t=calloc(1,sizeof(*t));t->fn=fn;t->arg=arg;assert(!pthread_create(&t->id,NULL,launch,t));return t;
}
Result threadJoin(Thread t,u64 timeout){
 assert(timeout==500000000LL);double end=seconds()+timeout/1e9;
 while(!atomic_load(&t->done)){if(seconds()>=end)return -1;usleep(1000);}
 return pthread_join(t->id,NULL);
}
void threadFree(Thread t){assert(atomic_load(&t->done));free(t);}
void threadDetach(Thread t){detached++;pthread_detach(t->id);}
Result srvIsServiceRegistered(bool *found,const char *name){assert(!strcmp(name,"fs:USER"));*found=true;return 0;}
void srvSetBlockingPolicy(bool value){assert(value);}
Result srvGetServiceHandleDirect(Handle *handle,const char *name){assert(!strcmp(name,"fs:USER"));*handle=42;return 0;}
Result FSUSER_Initialize(Handle handle){assert(handle==42);return 0;}
void fsUseSession(Handle handle){session=handle;}
void fsEndUseSession(void){assert(session==42);session=0;}
Result svcCloseHandle(Handle handle){assert(handle==42);closed++;return 0;}
FS_Path fsMakePath(int type,const void *path){assert(type==PATH_EMPTY||type==PATH_ASCII);return (FS_Path){path};}
Result FSUSER_OpenFileDirectly(Handle *file,int archive,FS_Path root,FS_Path path,u32 flags,u32 attrs){
 assert(session==42&&archive==ARCHIVE_SDMC&&!root.data[0]&&!strcmp(path.data,"/Folio/progress/123.state"));
 assert(flags==(FS_OPEN_WRITE|FS_OPEN_CREATE)&&attrs==0);*file=12;return mode==1?-1:0;
}
Result FSFILE_Write(Handle file,u32 *bytes,u64 offset,const void *data,u32 size,u32 flags){
 assert(session==42&&file==12&&!offset&&flags==FS_WRITE_FLUSH);writes++;
 if(mode==2)sleep(2);
 memcpy(output,data,size);*bytes=size;return 0;
}
Result FSFILE_SetSize(Handle file,u64 size){assert(file==12&&size==strlen(output));return 0;}
Result FSFILE_Close(Handle file){assert(file==12);return 0;}
int main(int argc,char **argv){
 mode=argc>1?atoi(argv[1]):0;
 assert(!shutdown_save_write("sdmc:/other.state","bad",3));
 const char *record="123\n45\n1\n999\n1234567\n";double before=seconds();
 bool ok=shutdown_save_write("sdmc:/Folio/progress/123.state",record,strlen(record));
 assert(seconds()-before<0.9);
 if(mode==0){assert(ok&&writes==1&&closed==1&&!detached&&!strcmp(record,output));}
 else if(mode==1){assert(!ok&&!writes&&closed==1&&!detached);}
 else {assert(!ok&&writes==1&&detached==1);}
 assert(!shutdown_save_write("sdmc:/Folio/progress/123.state",record,strlen(record)));
 puts("PASS: bounded private-session exit checkpoint");return 0;
}
