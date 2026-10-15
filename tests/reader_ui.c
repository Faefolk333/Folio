#include "reader.h"
#include "ui.h"
#include "i18n.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
static float measure(uint32_t cp,void *ctx){ReaderOptions *o=ctx;return (cp=='i'?.25f:.6f)*o->size;}
static void layout_checks(void){
 ReaderOptions o=reader_defaults();ReaderLayout l={0};
 const char *s="Příliš žluťoučký kůň úpěl ďábelské ódy. A long text with words that will wrap correctly.\n\nDalší kapitola.\n";
 assert(reader_layout(&l,s,strlen(s),&o,measure,&o));assert(l.count>=4);assert(l.rows==7);
 size_t anchor=strlen("Příliš žluťoučký kůň ");
 size_t line=reader_find_line(&l,anchor);assert(l.lines[line].start<=anchor&&l.lines[line].end>=anchor);
 for(size_t i=0;i<l.count;i++){
  assert(l.lines[i].width<=356.01f);assert(l.lines[i].start<=l.lines[i].end);
  assert(((unsigned char)s[l.lines[i].start]&0xc0)!=0x80);
  size_t end=i+1<l.count?l.lines[i+1].start:strlen(s);
  for(size_t j=l.lines[i].end;j<end;j++)assert(s[j]=='\r'||s[j]=='\n');
 }
 o.size=25;o.tracking=12;o.leading=205;
 assert(reader_layout(&l,s,strlen(s),&o,measure,&o));assert(l.rows==3);
 line=reader_find_line(&l,anchor);assert(l.lines[line].start<=anchor&&l.lines[line].end>=anchor);
 for(int visible=1;visible<=15;visible++){
  int focus=0;for(int i=0;i<30;i++)focus=reader_focus_move(focus,1,visible);assert(focus==visible-1);
  int active=0;for(int i=0;i<visible;i++)active+=i==focus;assert(active==1);
  assert(reader_focus_move(focus,-99,visible)==0);
 }
 assert(reader_focus_move(7,1,0)==0);assert(reader_visible(&l,l.count)==0);
 assert(reader_layout(&l,"",0,&o,measure,&o));assert(l.count==1);
 char longword[1000];memset(longword,'W',999);longword[999]=0;
 assert(reader_layout(&l,longword,999,&o,measure,&o));
 size_t total=0;for(size_t i=0;i<l.count;i++)total+=l.lines[i].end-l.lines[i].start;assert(total==999);
 assert(reader_bookmark_visible(&l,0,0));
 assert(reader_bookmark_visible(&l,0,l.lines[l.rows-1].start));
 assert(!reader_bookmark_visible(&l,0,l.lines[l.rows].start));
 assert(!reader_bookmark_visible(&l,1,0));
 assert(reader_bookmark_visible(&l,l.count-1,l.lines[l.count-1].start));
 assert(!reader_bookmark_visible(&l,l.count,0));
 size_t p=0;const char invalid[]="\xff\xc0\xf0\x80\x80\x80";while(p<sizeof(invalid)-1){size_t before=p;reader_codepoint(invalid,sizeof(invalid)-1,&p);assert(p>before);}
 assert(reader_bold_prefix(2,0)==0);assert(reader_bold_prefix(2,1)==2);assert(reader_bold_prefix(7,1)==3);assert(reader_bold_prefix(7,2)==4);
 reader_layout_free(&l);
}
static bool overlap(const UiControl *a,const UiControl *b){return a->x<b->x+b->w&&a->x+a->w>b->x&&a->y<b->y+b->h&&a->y+a->h>b->y;}
static void ui_checks(void){
 UiState s={.options=reader_defaults(),.hasBook=true,.hasBookmark=true,.listCount=12,.chapterCount=8,.pageCount=50,.currentPage=7,.currentChapter=2};UiControls c;
 bool seen[ACT_RESUME+1]={0};s.libraryReady=true;
 for(int lang=0;lang<2;lang++)for(int view=UI_HOME;view<=UI_FOLDER;view++)for(int mode=0;mode<2;mode++){
  s.language=lang;
  s.view=view;s.jumpMode=mode;s.options.focus=true;s.listPage=1;ui_build(&s,&c);
  for(int i=0;i<c.count;i++){
   UiControl *a=&c.controls[i];seen[a->action]=true;
   if(lang==LANG_EN){
    const char *labels[]={a->label,a->sub};
    for(int k=0;k<2;k++){size_t at=0,n=strlen(labels[k]);while(at<n){unsigned cp=reader_codepoint(labels[k],n,&at);assert(cp<0xc0||cp>0x17f);}}
   }
   assert(a->x>=0&&a->y>=0&&a->x+a->w<=320&&a->y+a->h<=240);
   if(a->enabled)assert(ui_hit(&c,a->x+a->w/2,a->y+a->h/2)==i);
   for(int j=i+1;j<c.count;j++)assert(!overlap(a,&c.controls[j]));
  }
 }
 for(int a=ACT_BACK;a<=ACT_RESUME;a++)if(a!=ACT_LIST_PAGE)assert(seen[a]);
 assert(ui_change(&s,ACT_SIZE,100));assert(s.options.size==25);
 ui_change(&s,ACT_SIZE,-100);assert(s.options.size==12);
 ui_change(&s,ACT_LEADING,1000);assert(s.options.leading==205);
 ui_change(&s,ACT_TRACKING,-1000);assert(s.options.tracking==-4);
 int f=s.options.font;ui_change(&s,ACT_FONT_CHANGE,1);assert(s.options.font!=(int)f);
 ui_change(&s,ACT_THEME,4);assert(s.options.theme==4);
 ui_change(&s,ACT_BIONIC,0);assert(s.options.bionic);
 s.focusLine=6;ui_change(&s,ACT_FOCUS,0);assert(s.focusLine==0);
 ui_change(&s,ACT_BRIGHTNESS,-100);assert(s.options.brightness==1);
 ui_change(&s,ACT_JUMP_MODE,1);assert(s.jumpValue==7);
 ui_change(&s,ACT_JUMP_STEP,100);assert(s.jumpValue==50);
 ui_change(&s,ACT_JUMP_MODE,0);assert(s.jumpValue==3);
 ui_change(&s,ACT_JUMP_STEP,-100);assert(s.jumpValue==1);
 ui_change(&s,ACT_FILTER,1);assert(s.listPage==0);
 s.selected=0;s.listCount=11;
 ui_move_entry(&s,1);assert(s.selected==1&&s.listPage==0);
 ui_move_entry(&s,3);assert(s.selected==4&&s.listPage==1);
 ui_move_entry(&s,100);assert(s.selected==10&&s.listPage==2);
 ui_move_entry(&s,-100);assert(s.selected==0&&s.listPage==0);
 s.listCount=0;ui_move_entry(&s,1);assert(s.selected==0);
 s.view=UI_LIBRARY;ui_change(&s,ACT_SETTINGS,0);ui_change(&s,ACT_BACK,0);assert(s.view==UI_LIBRARY);
 s.language=LANG_EN;ui_change(&s,ACT_LANGUAGE,0);assert(s.language==LANG_CS);
 assert(!strcmp(tr("Settings","Nastavení"),"Nastavení"));
 assert(!strcmp(i18n_error("Out of memory."),"Nedostatek paměti."));
 ui_change(&s,ACT_LANGUAGE,0);assert(s.language==LANG_EN);
 assert(!strcmp(i18n_error("Out of memory."),"Out of memory."));
 ui_change(&s,ACT_SLEEP,0);assert(s.sleeping);assert(!ui_wake(&s,false));assert(s.sleeping);
 assert(ui_wake(&s,true));assert(!s.sleeping);assert(!ui_wake(&s,true));
 s.view=UI_HOME;s.bookmarkHere=true;ui_build(&s,&c);
 for(int i=0;i<c.count;i++)if(c.controls[i].action==ACT_BOOKMARK)assert(!strcmp(c.controls[i].label,"Remove bookmark")&&c.controls[i].active);
 s.bookmarkHere=false;ui_build(&s,&c);
 for(int i=0;i<c.count;i++)if(c.controls[i].action==ACT_BOOKMARK)assert(!strcmp(c.controls[i].label,"Save bookmark")&&!c.controls[i].active);
 s.hasBook=false;s.view=UI_HOME;ui_build(&s,&c);
 for(int i=0;i<c.count;i++)if(c.controls[i].action==ACT_JUMP||c.controls[i].action==ACT_BOOKMARK)assert(!c.controls[i].enabled);
}
int main(void){layout_checks();ui_checks();puts("PASS: UTF-8 reflow, anchors, focus, Bionic, all touch controls, bounds and limits");return 0;}
