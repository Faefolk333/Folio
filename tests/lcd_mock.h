#pragma once
#include <stdbool.h>
typedef int Result;
#define R_FAILED(r) ((r)<0)
#define R_SUCCEEDED(r) ((r)>=0)
#define GSPLCD_SCREEN_BOTTOM 2
int envGetHandle(const char *name);
Result srvIsServiceRegistered(bool *found,const char *name);
void srvSetBlockingPolicy(bool nonBlocking);
Result gspLcdInit(void);
void gspLcdExit(void);
Result GSPLCD_PowerOffBacklight(unsigned screen);
Result GSPLCD_PowerOnBacklight(unsigned screen);
