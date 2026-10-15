#ifdef FOLIO_LCD_TEST
#include "../tests/lcd_mock.h"
#else
#include <3ds.h>
#endif
#include "backlight.h"
static bool initialized,unavailable,isOff,unknownState;
static bool ensure_service(void){
 if(initialized)return true;
 if(unavailable)return false;
 bool registered=false;
 if(!envGetHandle("gsp::Lcd")&&(R_FAILED(srvIsServiceRegistered(&registered,"gsp::Lcd"))||!registered)){
  unavailable=true;return false;
 }
 /* Never wait for a full service port, unlike the original startup attempt. */
 srvSetBlockingPolicy(true);
 Result result=gspLcdInit();
 srvSetBlockingPolicy(false);
 initialized=R_SUCCEEDED(result);unavailable=!initialized;return initialized;
}
bool backlight_sleep(bool sleeping){
 if(!sleeping&&!initialized)return true;
 if(sleeping&&!ensure_service())return false;
 if(isOff==sleeping&&!unknownState)return true;
 Result result=sleeping?GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTTOM):GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM);
 if(R_FAILED(result))return false;
 isOff=sleeping;unknownState=false;return true;
}
void backlight_restore(void){if(initialized&&(isOff||unknownState))backlight_sleep(false);}
void backlight_resume(void){if(initialized)unknownState=true;}
void backlight_exit(void){backlight_restore();if(initialized)gspLcdExit();initialized=false;unavailable=false;isOff=false;unknownState=false;}
