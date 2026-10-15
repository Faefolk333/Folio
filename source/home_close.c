#ifdef FOLIO_HOME_CLOSE_TEST
#include "../tests/home_close_mock.h"
#else
#include <3ds.h>
#endif
#include <stdatomic.h>
#include "home_close.h"

enum { WATCHING, MAIN_EXIT, SYSTEM_EXIT, CLOSE_FAILED };
static atomic_int owner;
static atomic_bool suspended;
static Thread watcher;
static void system_close(void){
 Result rc=APT_PrepareToCloseApplication(true);
 if(R_SUCCEEDED(rc))rc=APT_CloseApplication(NULL,0,0);
 if(R_SUCCEEDED(rc))svcExitProcess();
}

static void watch(void *unused){
 (void)unused;unsigned pending=0;
 while(atomic_load(&owner)==WATCHING){
  svcSleepThread(50000000LL);
  /* APT's notification thread can set ORDERTOCLOSE while the main thread is
     still blocked in aptMainLoop's wait-for-wakeup. HOME alone is not a close.
     Never acquire graphics or re-enter aptMainLoop from this worker. */
  if(!atomic_load(&suspended)||aptIsActive()||!aptShouldClose()){pending=0;continue;}
  if(++pending<5)continue;
  int expected=WATCHING;
  if(!atomic_compare_exchange_strong(&owner,&expected,SYSTEM_EXIT))break;
  /* Same application-close commands used by libctru aptExit. A system-requested
     close while suspended needs no graphics reacquisition or GPU teardown.
     All owned handles/memory are reclaimed by process termination. */
  system_close();
  atomic_store(&owner,CLOSE_FAILED);
  break;
 }
}
void home_close_finish(void){
 home_close_stop();
 /* APTCMD_WAKEUP_CANCEL can return to main without restoring GPU rights.
    Do not enter citro/gfx teardown in that suspended-close case either. */
 if(!envIsHomebrew()&&atomic_load(&suspended)&&aptShouldClose())system_close();
}

bool home_close_start(void){
 if(envIsHomebrew())return true;
 watcher=threadCreate(watch,NULL,8192,0x32,0,false);
 return watcher!=NULL;
}
void home_close_suspend(bool value){atomic_store(&suspended,value);}
void home_close_stop(void){
 int expected=WATCHING;
 atomic_compare_exchange_strong(&owner,&expected,MAIN_EXIT);
 /* No concurrent aptExit/APT_CloseApplication. The worker only owns exit after
    an explicit system close while suspended. Its command normally kills us. */
 while(atomic_load(&owner)==SYSTEM_EXIT)svcSleepThread(1000000LL);
 if(watcher){
  if(R_SUCCEEDED(threadJoin(watcher,0)))threadFree(watcher);
  else threadDetach(watcher);
  watcher=NULL;
 }
}
