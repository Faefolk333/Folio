#ifdef FOLIO_SHUTDOWN_TEST
#include "../tests/shutdown_mock.h"
#else
#include <3ds.h>
#endif
#include <stdatomic.h>
#include <string.h>
#include "folio.h"
#include "shutdown_save.h"

static char savedPath[FOLIO_PATH_MAX], savedRecord[128];
static size_t savedLength;
static bool started;
static atomic_bool succeeded;

static void write_checkpoint(void *unused) {
 (void)unused;
 Handle session=0,file=0;bool registered=false;u32 bytes=0;
 Result rc=srvIsServiceRegistered(&registered,"fs:USER");
 if(R_FAILED(rc)||!registered)return;
 /* Non-blocking service acquisition and thread-local session override. Never
    use newlib stdio or the main thread's shared SD archive during shutdown. */
 srvSetBlockingPolicy(true);
 rc=srvGetServiceHandleDirect(&session,"fs:USER");
 if(R_FAILED(rc))return;
 rc=FSUSER_Initialize(session);
 if(R_SUCCEEDED(rc)){
  fsUseSession(session);
  rc=FSUSER_OpenFileDirectly(&file,ARCHIVE_SDMC,fsMakePath(PATH_EMPTY,""),
      fsMakePath(PATH_ASCII,savedPath+5),FS_OPEN_WRITE|FS_OPEN_CREATE,0);
  if(R_SUCCEEDED(rc)){
   rc=FSFILE_Write(file,&bytes,0,savedRecord,(u32)savedLength,FS_WRITE_FLUSH);
   if(R_SUCCEEDED(rc)&&bytes==savedLength)rc=FSFILE_SetSize(file,savedLength);
   Result closed=FSFILE_Close(file);
   atomic_store(&succeeded,R_SUCCEEDED(rc)&&R_SUCCEEDED(closed)&&bytes==savedLength);
  }
  fsEndUseSession();
 }
 svcCloseHandle(session);
}

bool shutdown_save_write(const char *path,const char *record,size_t length){
 static const char prefix[]="sdmc:/Folio/progress/";
 if(started||!path||!record||!length||length>=sizeof(savedRecord)||
    strlen(path)>=sizeof(savedPath)||strncmp(path,prefix,sizeof(prefix)-1))return false;
 started=true;
 strcpy(savedPath,path);memcpy(savedRecord,record,length);savedLength=length;
 atomic_store(&succeeded,false);
 Thread worker=threadCreate(write_checkpoint,NULL,16*1024,0x31,0,false);
 if(!worker){started=false;return false;}
 if(R_FAILED(threadJoin(worker,500000000LL))){
  /* Do not free a live thread or its snapshot; no second exit job is allowed. */
  threadDetach(worker);return false;
 }
 bool result=atomic_load(&succeeded);threadFree(worker);started=false;return result;
}
