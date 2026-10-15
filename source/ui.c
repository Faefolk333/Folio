#include "ui.h"
#include "i18n.h"
#include <stdio.h>
#include <string.h>
static int clamp(int v,int lo,int hi){if(hi<lo)hi=lo;return v<lo?lo:v>hi?hi:v;}
bool ui_wake(UiState *s,bool input){if(!s->sleeping||!input)return false;s->sleeping=false;return true;}
void ui_move_entry(UiState *s,int delta){s->selected=clamp(s->selected+delta,0,s->listCount?s->listCount-1:0);s->listPage=s->selected/4;}
static UiControl *add(UiControls *u,int x,int y,int w,int h,UiAction a,int v,const char *label,const char *sub,int style,bool active,bool enabled){
 if(u->count>=40)return NULL;
 UiControl *c=&u->controls[u->count++];
 *c=(UiControl){.x=x,.y=y,.w=w,.h=h,.action=a,.value=v,.style=style,.active=active,.enabled=enabled};
 snprintf(c->label,sizeof(c->label),"%s",label);snprintf(c->sub,sizeof(c->sub),"%s",sub?sub:"");return c;
}
#define BUTTON(x,y,w,h,a,v,s) add(u,x,y,w,h,a,v,s,NULL,0,false,true)
void ui_build(const UiState *s,UiControls *u){
 i18n_set(s->language);
 memset(u,0,sizeof(*u));char text[112];const ReaderOptions *o=&s->options;
 if(s->view==UI_HOME){
  BUTTON(280,8,30,28,ACT_SLEEP,0,"Zz");
  add(u,10,44,146,65,ACT_DISPLAY,0,tr("Display","Displej"),tr("Theme and colours","Téma a barvy"),1,false,true);
  add(u,164,44,146,65,ACT_FONT,0,tr("Type","Písmo"),tr("Font and typography","Font a typografie"),1,false,true);
  add(u,10,116,300,39,ACT_SETTINGS,0,tr("Settings","Nastavení"),tr("Bionic Reading and Focus Line","Bionic Reading a Focus Line"),2,false,true);
  add(u,10,162,146,30,ACT_JUMP,0,tr("Go to…","Přejít na…"),NULL,0,false,s->hasBook);
  add(u,164,162,146,30,ACT_BOOKMARK,0,s->bookmarkHere?tr("Remove bookmark","Smazat záložku"):tr("Save bookmark","Uložit záložku"),NULL,0,s->bookmarkHere,s->hasBook);
 }else{
  BUTTON(10,8,30,28,ACT_BACK,0,"<");
 }
 if(s->view==UI_DISPLAY){
  const char *names[]={tr("Light","Světlé"),tr("Warm","Teplé"),tr("Paper","Papír"),tr("Dark","Tmavé"),tr("Red Night","Červená noc")};
  for(int i=0;i<5;i++)add(u,18+(i%3)*98,62+(i/3)*60,90,52,ACT_THEME,i,names[i],NULL,3,o->theme==i,true);
 }else if(s->view==UI_FONT){
  const char *fonts[]={"Noto Sans","Noto Serif",tr("System","Systémový"),"Inter","EB Garamond","Folio Baskerville"};
  snprintf(text,sizeof(text),"Font: %s  >",fonts[o->font]);
  BUTTON(10,44,300,28,ACT_FONT_CHANGE,1,text);
  BUTTON(10,80,44,30,ACT_SIZE,-1,"A-");
  BUTTON(266,80,44,30,ACT_SIZE,1,"A+");
  BUTTON(10,136,30,28,ACT_LEADING,-8,"-");
  BUTTON(126,136,30,28,ACT_LEADING,8,"+");
  BUTTON(164,136,30,28,ACT_TRACKING,-1,"-");
  BUTTON(280,136,30,28,ACT_TRACKING,1,"+");
 }else if(s->view==UI_SETTINGS){
  add(u,18,66,64,28,ACT_BIONIC,0,"Bionic",NULL,0,o->bionic,true);
  add(u,86,66,64,28,ACT_FOCUS,0,"Focus",NULL,0,o->focus,true);
  const char *strength[]={tr("Light emphasis","Nízké zvýraznění"),tr("Medium emphasis","Střední zvýraznění"),tr("Strong emphasis","Silné zvýraznění")};
  BUTTON(18,100,132,26,ACT_STRENGTH,1,strength[o->strength]);
  add(u,174,66,58,28,ACT_FOCUS_MOVE,-1,tr("Up","Nahoru"),NULL,0,false,o->focus);
  add(u,238,66,58,28,ACT_FOCUS_MOVE,1,tr("Down","Dolů"),NULL,0,false,o->focus);
  snprintf(text,sizeof(text),o->sleepSeconds?tr("Sleep after %d s","Zhasnout za %d s"):tr("Never sleep","Bez zhasínání"),o->sleepSeconds);
  BUTTON(174,161,124,26,ACT_SLEEP_TIME,1,text);
  BUTTON(18,162,132,25,ACT_LANGUAGE,0,s->language==LANG_CS?"Čeština >":"English >");
 }else if(s->view==UI_LIBRARY){
  BUTTON(224,8,86,28,ACT_SETTINGS,0,tr("Settings","Nastavení"));
  const char *filters[]={tr("All","Vše"),"EPUB","TXT","HTML"};
  for(int i=0;i<4;i++)add(u,10+i*76,43,72,25,ACT_FILTER,i,filters[i],NULL,0,s->filter==i,true);
  for(int row=0;row<4;row++)if(s->listPage*4+row<s->listCount)
   add(u,10,75+row*29,300,26,ACT_ENTRY,s->listPage*4+row,"",NULL,0,s->selected==s->listPage*4+row,true);
  BUTTON(10,200,130,30,ACT_CHOOSE_FOLDER,0,tr("Books folder","Složka knih"));
  BUTTON(148,200,76,30,ACT_SCAN,0,tr("Refresh","Obnovit"));
  add(u,232,200,78,30,ACT_RESUME,0,tr("Read","Číst"),NULL,0,false,s->hasBook);
 }else if(s->view==UI_FOLDER){
  BUTTON(224,8,86,28,ACT_SCAN,0,tr("Refresh","Obnovit"));
  for(int row=0;row<4;row++)if(s->listPage*4+row<s->listCount)
   add(u,10,75+row*29,300,26,ACT_ENTRY,s->listPage*4+row,"",NULL,0,s->selected==s->listPage*4+row,true);
  BUTTON(10,200,100,30,ACT_PARENT,0,tr("Parent","O úroveň výš"));
  add(u,118,200,192,30,ACT_USE_FOLDER,0,tr("Use this folder","Použít tuto složku"),NULL,0,false,s->libraryReady);
 }else if(s->view==UI_JUMP){
  add(u,10,44,146,28,ACT_JUMP_MODE,0,tr("Chapter","Kapitola"),NULL,0,s->jumpMode==0,true);
  add(u,164,44,146,28,ACT_JUMP_MODE,1,tr("Page","Stránka"),NULL,0,s->jumpMode==1,true);
  BUTTON(10,84,42,34,ACT_JUMP_STEP,-1,"-");
  BUTTON(268,84,42,34,ACT_JUMP_STEP,1,"+");
  if(s->jumpMode)BUTTON(60,84,200,34,ACT_PAGE_INPUT,0,"");
  add(u,10,145,300,29,ACT_BOOKMARK_GO,0,tr("Go to saved bookmark","Přejít na uloženou záložku"),NULL,0,false,s->hasBookmark);
  BUTTON(10,199,146,31,ACT_BACK,0,tr("Cancel","Zrušit"));
  add(u,164,199,146,31,ACT_JUMP_CONFIRM,0,tr("Go","Přejít"),NULL,0,true,s->hasBook);
 }
 if(s->view<=UI_SETTINGS){
  BUTTON(10,200,92,30,ACT_LIBRARY,0,tr("Library","Knihovna"));
  BUTTON(116,202,28,26,ACT_BRIGHTNESS,-1,"-");
  BUTTON(276,202,28,26,ACT_BRIGHTNESS,1,"+");
 }
}
int ui_hit(const UiControls *u,int x,int y){
 for(int i=0;i<u->count;i++){const UiControl *c=&u->controls[i];
  if(c->enabled&&x>=c->x&&x<c->x+c->w&&y>=c->y&&y<c->y+c->h)return i;}
 return -1;
}
bool ui_change(UiState *s,UiAction a,int v){
 ReaderOptions *o=&s->options;
 switch(a){
 case ACT_BACK:s->view=s->view==UI_FOLDER?UI_LIBRARY:s->view==UI_SETTINGS?s->settingsReturn:UI_HOME;break;
 case ACT_SLEEP:s->sleeping=true;break;
 case ACT_DISPLAY:s->view=UI_DISPLAY;break;
 case ACT_FONT:s->view=UI_FONT;break;
 case ACT_SETTINGS:s->settingsReturn=s->view;s->view=UI_SETTINGS;break;
 case ACT_LIBRARY:s->view=UI_LIBRARY;break;
 case ACT_BRIGHTNESS:o->brightness=clamp(o->brightness+v,1,5);break;
 case ACT_THEME:o->theme=clamp(v,0,4);break;
 case ACT_FONT_CHANGE:o->font=(o->font+v+READER_FONT_COUNT)%READER_FONT_COUNT;return true;
 case ACT_SIZE:o->size=clamp(o->size+v,12,25);return true;
 case ACT_LEADING:o->leading=clamp(o->leading+v,115,205);return true;
 case ACT_TRACKING:o->tracking=clamp(o->tracking+v,-4,12);return true;
 case ACT_BIONIC:o->bionic=!o->bionic;break;
 case ACT_FOCUS:o->focus=!o->focus;s->focusLine=0;break;
 case ACT_STRENGTH:o->strength=(o->strength+1)%3;break;
 case ACT_SLEEP_TIME:o->sleepSeconds=o->sleepSeconds==0?20:o->sleepSeconds==20?40:o->sleepSeconds==40?60:0;break;
 case ACT_FILTER:s->filter=clamp(v,0,3);s->selected=s->listPage=0;break;
 case ACT_LIST_PAGE:ui_move_entry(s,v*4);break;
 case ACT_LANGUAGE:s->language=(s->language+1)%LANG_COUNT;i18n_set(s->language);break;
 case ACT_CHOOSE_FOLDER:s->view=UI_FOLDER;s->selected=s->listPage=0;break;
 case ACT_RESUME:s->view=UI_HOME;break;
 case ACT_JUMP:s->view=UI_JUMP;s->jumpValue=s->jumpMode?s->currentPage:s->currentChapter+1;break;
 case ACT_JUMP_MODE:s->jumpMode=v;s->jumpValue=v?s->currentPage:s->currentChapter+1;break;
 case ACT_JUMP_STEP:s->jumpValue=clamp(s->jumpValue+v,1,s->jumpMode?s->pageCount:s->chapterCount);break;
 default:break;
 }
 return false;
}
