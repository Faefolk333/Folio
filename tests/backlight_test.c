#include "lcd_mock.h"
#include "backlight.h"
#include <assert.h>
#include <stdio.h>
static int initCount,exitCount,offCount,onCount;static bool registered=true,policy;
static int initResult,offResult;
int envGetHandle(const char *name){(void)name;return 0;}
Result srvIsServiceRegistered(bool *found,const char *name){(void)name;*found=registered;return 0;}
void srvSetBlockingPolicy(bool nonBlocking){policy=nonBlocking;}
Result gspLcdInit(void){assert(policy);initCount++;return initResult;}
void gspLcdExit(void){exitCount++;}
Result GSPLCD_PowerOffBacklight(unsigned screen){assert(screen==2);offCount++;return offResult;}
Result GSPLCD_PowerOnBacklight(unsigned screen){assert(screen==2);onCount++;return 0;}
int main(void){
 assert(backlight_sleep(false));assert(initCount==0);
 assert(backlight_sleep(true));assert(initCount==1&&offCount==1&&!policy);
 assert(backlight_sleep(true));assert(offCount==1);
 backlight_restore();assert(onCount==1);
 backlight_resume();assert(backlight_sleep(true));assert(offCount==2);
 assert(backlight_sleep(false));assert(onCount==2);
 assert(backlight_sleep(true));backlight_exit();assert(onCount==3&&exitCount==1);
 registered=false;assert(!backlight_sleep(true));assert(initCount==1);backlight_exit();
 registered=true;initResult=-1;assert(!backlight_sleep(true));assert(!policy);backlight_exit();
 initResult=0;offResult=-1;assert(!backlight_sleep(true));backlight_exit();
 puts("PASS: lazy nonblocking LCD init, bottom-only commands, sleep/wake/exit restore, errors");return 0;
}
