#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef int Result;
typedef void *Thread;
#define R_SUCCEEDED(rc) ((rc)>=0)
bool envIsHomebrew(void);
bool aptIsActive(void);
bool aptShouldClose(void);
void svcSleepThread(int64_t ns);
void svcExitProcess(void);
Result APT_PrepareToCloseApplication(bool cancel);
Result APT_CloseApplication(const void *param,size_t size,int handle);
Thread threadCreate(void (*fn)(void*),void *arg,size_t stack,int priority,int core,bool detached);
Result threadJoin(Thread thread,uint64_t timeout);
void threadFree(Thread thread);
void threadDetach(Thread thread);
