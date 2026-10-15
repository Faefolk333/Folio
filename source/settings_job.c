#ifdef FOLIO_JOB_TEST
#include "../tests/settings_job_mock.h"
#else
#include <3ds.h>
#endif
#include <stdatomic.h>
#include <string.h>
#include "settings_job.h"
static Thread worker;
static FolioSettings snapshot;
static atomic_int finished,stage,jobErrorCode;
static bool saved;
/* FS session override is thread-local. No newlib FILE objects, global stdio
   locks, or shared SD device archive are used by the saving thread. */
static Handle privateSession;
bool settings_io_active(void){return threadGetCurrent()!=NULL&&privateSession!=0;}
static bool file_path(const char *path){
 return !strcmp(path,"sdmc:/Folio/preferences0.cfg")||!strcmp(path,"sdmc:/Folio/preferences1.cfg");
}
bool settings_io_read(const char *path,void *data,size_t size){
 if(!file_path(path))return false;
 Handle file=0;u32 bytes=0;atomic_store(&stage,3);
 Result rc=FSUSER_OpenFileDirectly(&file,ARCHIVE_SDMC,fsMakePath(PATH_EMPTY,""),fsMakePath(PATH_ASCII,path+5),FS_OPEN_READ,0);
 if(R_FAILED(rc))return false; /* An absent slot is normal on the first save. */
 atomic_store(&stage,4);rc=FSFILE_Read(file,&bytes,0,data,(u32)size);
 atomic_store(&stage,5);Result closeResult=FSFILE_Close(file);
 return R_SUCCEEDED(rc)&&R_SUCCEEDED(closeResult)&&bytes==size;
}
bool settings_io_write(const char *path,const void *data,size_t size){
 if(!file_path(path))return false;
 Handle file=0;u32 bytes=0;atomic_store(&stage,6);
 Result rc=FSUSER_OpenFileDirectly(&file,ARCHIVE_SDMC,fsMakePath(PATH_EMPTY,""),fsMakePath(PATH_ASCII,path+5),FS_OPEN_WRITE|FS_OPEN_CREATE,0);
 if(R_FAILED(rc)){atomic_store(&jobErrorCode,rc);return false;}
 /* Fixed-size records overwrite only the alternate slot, never book data. */
 atomic_store(&stage,7);rc=FSFILE_Write(file,&bytes,0,data,(u32)size,FS_WRITE_FLUSH);
 atomic_store(&stage,8);Result closeResult=FSFILE_Close(file);
 if(R_FAILED(rc))atomic_store(&jobErrorCode,rc);
 else if(R_FAILED(closeResult))atomic_store(&jobErrorCode,closeResult);
 else if(bytes!=size)atomic_store(&jobErrorCode,-1);
 return R_SUCCEEDED(rc)&&R_SUCCEEDED(closeResult)&&bytes==size;
}
static void run(void *unused){
 (void)unused;Handle session=0;bool registered=false;
 atomic_store(&stage,1);
 Result rc=srvIsServiceRegistered(&registered,"fs:USER");
 if(R_SUCCEEDED(rc)&&registered){
  srvSetBlockingPolicy(true);
  /* Direct acquisition is intentional: do not duplicate the launcher's
     shared FS session, whose blocked operation could also block reading. */
  rc=srvGetServiceHandleDirect(&session,"fs:USER");
 }
 else if(R_SUCCEEDED(rc))rc=-1;
 if(R_SUCCEEDED(rc)){
  atomic_store(&stage,2);rc=FSUSER_Initialize(session);
 }
 if(R_SUCCEEDED(rc)){
  privateSession=session;fsUseSession(session);
  if(!snapshot.generation&&!snapshot.folder[0]){
   FolioSettings previous;
   if(settings_load("sdmc:/Folio",&previous)){snapshot.generation=previous.generation;strcpy(snapshot.folder,previous.folder);}
  }
  saved=settings_save("sdmc:/Folio",&snapshot);
  fsEndUseSession();privateSession=0;
 }else atomic_store(&jobErrorCode,rc);
 if(session){atomic_store(&stage,9);svcCloseHandle(session);}
 atomic_store_explicit(&finished,1,memory_order_release);
}
bool settings_job_start(const FolioSettings *value){
 if(worker)return false;
 snapshot=*value;saved=false;atomic_store(&finished,0);atomic_store(&stage,0);atomic_store(&jobErrorCode,0);
 /* The UI is the main thread (threadGetCurrent()==NULL); worker I/O can
    begin safely even before threadCreate returns its handle to the caller. */
 worker=threadCreate(run,NULL,16*1024,0x31,0,false);
 return worker!=NULL;
}
bool settings_job_busy(void){return worker!=NULL;}
int settings_job_poll(FolioSettings *out){
 if(!worker||!atomic_load_explicit(&finished,memory_order_acquire))return 0;
 if(R_FAILED(threadJoin(worker,0)))return 0;
 int result=saved?1:-1;if(saved)*out=snapshot;
 threadFree(worker);worker=NULL;return result;
}
unsigned settings_job_stage(void){return (unsigned)atomic_load(&stage);}
int32_t settings_job_error(void){return atomic_load(&jobErrorCode);}
void settings_job_shutdown(void){
 if(!worker)return;
 /* Never join a stalled filesystem call. Do not free memory still in use.
    The worker has private handles and no dependency on graphics or stdio.
    Normal application teardown terminates remaining work. */
 if(R_SUCCEEDED(threadJoin(worker,0)))threadFree(worker);
 else threadDetach(worker);
 worker=NULL;
}
