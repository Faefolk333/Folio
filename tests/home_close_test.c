#define FOLIO_HOME_CLOSE_TEST
#include "../source/home_close.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static int mode,ticks,prepare,closeCalls,exited,created;
bool envIsHomebrew(void){return mode==6;}
bool aptIsActive(void){return mode==2||mode==8;}
bool aptShouldClose(void){return mode!=0&&(mode!=7||ticks<=2||ticks>=20);}
void svcSleepThread(int64_t ns){
 assert(ns==50000000LL);ticks++;
 if(ticks>=30)atomic_store(&owner,MAIN_EXIT);
}
void svcExitProcess(void){exited++;}
Result APT_PrepareToCloseApplication(bool cancel){assert(cancel);prepare++;return mode==4?-1:0;}
Result APT_CloseApplication(const void *param,size_t size,int handle){assert(!param&&!size&&!handle);closeCalls++;return 0;}
Thread threadCreate(void(*fn)(void*),void *arg,size_t stack,int priority,int core,bool detached){
 assert(fn==watch&&!arg&&stack==8192&&priority==0x32&&core==0&&!detached);created++;return (Thread)1;
}
Result threadJoin(Thread thread,uint64_t timeout){assert(thread==(Thread)1&&!timeout);return 0;}
void threadFree(Thread thread){assert(thread==(Thread)1);}
void threadDetach(Thread thread){(void)thread;assert(0);}
int main(int argc,char **argv){
 mode=argc>1?atoi(argv[1]):0;
 assert(home_close_start());home_close_suspend(mode!=3);
 if(mode==5)home_close_stop();
 if(mode==8)home_close_finish();
 else if(mode!=6)watch(NULL);
 home_close_stop();
 if(mode==1||mode==7||mode==8){assert(prepare==1&&closeCalls==1&&exited==1);if(mode!=8)assert(ticks>=(mode==7?24:5));}
 else if(mode==4){assert(prepare==1&&!closeCalls&&!exited);}
 else assert(!prepare&&!closeCalls&&!exited);
 assert(created==(mode==6?0:1));
 puts("PASS: HOME close guard ownership, explicit-close gate, grace period and HBL exclusion");
}
