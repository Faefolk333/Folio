#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef int Result;typedef int Handle;typedef uint32_t u32;typedef uint64_t u64;
typedef struct MockThread *Thread;
typedef struct {const char *data;} FS_Path;
#define R_FAILED(v) ((v)<0)
#define R_SUCCEEDED(v) ((v)>=0)
#define ARCHIVE_SDMC 9
#define PATH_EMPTY 1
#define PATH_ASCII 3
#define FS_OPEN_READ 1
#define FS_OPEN_WRITE 2
#define FS_OPEN_CREATE 4
#define FS_WRITE_FLUSH 1
Thread threadCreate(void (*fn)(void*),void *arg,size_t stack,int priority,int core,bool detached);
Thread threadGetCurrent(void);
Result threadJoin(Thread t,u64 timeout);
void threadFree(Thread t);
void threadDetach(Thread t);
Result srvIsServiceRegistered(bool *registered,const char *name);
void srvSetBlockingPolicy(bool value);
Result srvGetServiceHandleDirect(Handle *handle,const char *name);
Result FSUSER_Initialize(Handle handle);
void fsUseSession(Handle session);
void fsEndUseSession(void);
Result svcCloseHandle(Handle session);
FS_Path fsMakePath(int type,const void *path);
Result FSUSER_OpenFileDirectly(Handle *file,int archive,FS_Path root,FS_Path path,u32 flags,u32 attrs);
Result FSFILE_Read(Handle file,u32 *bytes,u64 offset,void *data,u32 size);
Result FSFILE_Write(Handle file,u32 *bytes,u64 offset,const void *data,u32 size,u32 flags);
Result FSFILE_Close(Handle file);
