#ifdef FOLIO_HOST_PREVIEW
#include "../tests/preview_platform.h"
#else
#include <3ds.h>
#include <citro2d.h>
#endif
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include "folio.h"
#include "reader.h"
#include "ui.h"
#include "startup.h"
#include "i18n.h"
#include "settings.h"
#include "backlight.h"
#ifndef FOLIO_HOST_PREVIEW
#include "settings_job.h"
#endif

/* No mkdir and no SD/LCD service work before the first rendered frame. */
#define PROGRESS_DIR "sdmc:/Folio/progress"
typedef struct {unsigned paper,ink,muted,bg,card,card2,border,fg,secondary,accent,onAccent;} Palette;
static const Palette palettes[]={
 {0xfafafa,0x1f1f1f,0x858585,0xf3f4f5,0xffffff,0xeceff1,0xd6d9dc,0x1c1f22,0x767d84,0x22262b,0xffffff},
 {0xf3e3c3,0x28241e,0x827a6d,0x191c20,0x25292e,0x2c3137,0x3e444b,0xf5f5f3,0x9aa0a6,0xece6d8,0x17191c},
 {0xe4c89b,0x2d251c,0x7b6d5a,0xd6bd95,0xcbbb9f,0xbfae91,0x9c8a6c,0x2a241d,0x655b4d,0x332a21,0xefe2c8},
 {0x0f1113,0xd9dcdf,0x747b82,0x0d1012,0x171a1e,0x1f2328,0x30363d,0xeef0f2,0x7f878f,0xe3e5e7,0x14171a},
 {0x070303,0xb94a46,0x6d2c2a,0x120606,0x1c0a0a,0x260d0d,0x4b1717,0xd76862,0x8e3d39,0x7a1d1a,0xffd4d0}
};
typedef struct {char name[128],path[FOLIO_PATH_MAX];bool directory;int percent;uint64_t lastRead;} Entry;
static int compare_entries(const void *a,const void *b){
 const Entry *x=a,*y=b;if(x->directory!=y->directory)return x->directory?-1:1;
 if(!x->directory&&x->lastRead!=y->lastRead)return x->lastRead>y->lastRead?-1:1;
 int result=strcasecmp(x->name,y->name);return result?result:strcmp(x->path,y->path);
}
static C3D_RenderTarget *top,*bottom;
static C2D_TextBuf textBuf;
static C2D_Font fonts[READER_FONT_COUNT];
extern const unsigned char folio_sans_start[],folio_sans_end[],folio_serif_start[],folio_serif_end[];
extern const unsigned char folio_inter_start[],folio_inter_end[],folio_garamond_start[],folio_garamond_end[],folio_baskerville_start[],folio_baskerville_end[];
static UiState ui;
static UiControls controls;
static ReaderLayout layout;
static FolioDocument doc;
static size_t position,bookmark,firstLine;
static uint64_t currentLastRead,latestRead;
static bool dirty,saveFailed;
static bool configFailed,lcdFailed;
static FolioSettings preferences;
static bool preferencesQueued,preferencesSaving;
static unsigned preferenceStage;
static int32_t preferenceError;
static u64 preferencesStarted;
static char currentPath[FOLIO_PATH_MAX],folder[FOLIO_PATH_MAX]="sdmc:/",status[160]="Choose your books folder.";
static Entry entries[256];
static int entryCount,filtered[256],focused=-1;
static u64 lastInput;
static const char *welcome(void){return tr(
 "Welcome to Folio.\n\nChoose your books folder in Library. It will refresh on your next launch.\n\nChange the display, type and reading style below. L and R turn pages.\n\nA quiet place for your books.",
 "Vítej ve Folio.\n\nV knihovně vyber složku knih. Při dalším startu se sama obnoví.\n\nDisplej, písmo i způsob čtení si uprav dole. L a R otáčejí stránky.\n\nKlidné místo pro tvoje knihy.");}

static u32 color(unsigned hex){return C2D_Color32(hex>>16,(hex>>8)&255,hex&255,255);}
static unsigned blend(unsigned ink,unsigned bg,float a){
 unsigned r=(unsigned)(((ink>>16)&255)*a+((bg>>16)&255)*(1-a));
 unsigned g=(unsigned)(((ink>>8)&255)*a+((bg>>8)&255)*(1-a));
 unsigned b=(unsigned)((ink&255)*a+(bg&255)*(1-a));return (r<<16)|(g<<8)|b;
}
static void rect(float x,float y,float w,float h,unsigned c){C2D_DrawRectSolid(x,y,0,w,h,color(c));}
static void roundrect(float x,float y,float w,float h,float r,unsigned c){
 rect(x+r,y,w-2*r,h,c);rect(x,y+r,w,h-2*r,c);
 C2D_DrawEllipseSolid(x,y,0,2*r,2*r,color(c));C2D_DrawEllipseSolid(x+w-2*r,y,0,2*r,2*r,color(c));
 C2D_DrawEllipseSolid(x,y+h-2*r,0,2*r,2*r,color(c));C2D_DrawEllipseSolid(x+w-2*r,y+h-2*r,0,2*r,2*r,color(c));
}
static float scale(C2D_Font font,float size){FINF_s *f=C2D_FontGetInfo(font);return size/(f->height?f->height:30);}
static float advance(C2D_Font font,uint32_t cp,float size){
 if(cp=='\t')cp=' ';
 int glyph=C2D_FontGlyphIndexFromCodePoint(font,cp);
 return C2D_FontGetCharWidthInfo(font,glyph)->charWidth*scale(font,size);
}
static void text(C2D_Font font,float x,float y,float size,unsigned c,const char *s){
 C2D_Text t;C2D_TextFontParse(&t,font,textBuf,s);
 /* citro2d normalizes every atlas cell to 30 px internally. Cancel that
    normalization so measured raw advances and drawn glyphs agree. */
 float k=scale(font,size)*C2D_FontGetInfo(font)->tglp->cellHeight/30.f;
 C2D_DrawText(&t,C2D_WithColor|C2D_AtBaseline,x,y+size*.82f,0,k,k,color(c));
}
static void label(float x,float y,float w,float size,unsigned c,const char *s,bool centered){
 char fit[256];size_t n=strlen(s),p=0,used=0;float len=0,ellipsis=advance(fonts[0],0x2026,size);
 while(p<n){size_t before=p;uint32_t cp=reader_codepoint(s,n,&p);float step=advance(fonts[0],cp,size);
  if(p>=sizeof(fit)-4||len+step+(p<n?ellipsis:0)>w){p=before;break;}
  memcpy(fit+used,s+before,p-before);used+=p-before;len+=step;
 }
 if(p<n&&used+3<sizeof(fit)){memcpy(fit+used,"…",3);used+=3;len+=ellipsis;}
 fit[used]=0;text(fonts[0],x+(centered?(w-len)/2:0),y,size,c,fit);
}
/* Cache the common Latin/Greek/Cyrillic advances across whole-book layouts. */
static float measure(uint32_t cp,void *ctx){
 (void)ctx;static float widths[2048];static unsigned char valid[2048];
 static int cachedFont=-1,cachedSize=-1;
 if(cachedFont!=ui.options.font||cachedSize!=ui.options.size){
  memset(valid,0,sizeof(valid));cachedFont=ui.options.font;cachedSize=ui.options.size;
 }
 if(cp>=2048)return advance(fonts[ui.options.font],cp,ui.options.size);
 if(!valid[cp]){widths[cp]=advance(fonts[ui.options.font],cp,ui.options.size);valid[cp]=1;}
 return widths[cp];
}
static const char *booktext(void){return doc.text?doc.text:welcome();}
static size_t booklength(void){return doc.text?doc.length:strlen(welcome());}
static void sync_position(void){
 firstLine=reader_find_line(&layout,position);
 ui.bookmarkHere=ui.hasBookmark&&reader_bookmark_visible(&layout,firstLine,bookmark);
 ui.hasBook=doc.text!=NULL;ui.chapterCount=doc.chapterCount?doc.chapterCount:1;
 ui.currentChapter=0;
 for(int i=0;i<doc.chapterCount;i++)if(doc.chapters[i].offset<=position)ui.currentChapter=i;
 ui.pageCount=layout.rows?(int)((layout.count+layout.rows-1)/layout.rows):1;
 ui.currentPage=layout.rows?(int)(firstLine/layout.rows)+1:1;
 ui.focusLine=reader_focus_move(ui.focusLine,0,reader_visible(&layout,firstLine));
}
static bool reflow(void){
 if(!reader_layout(&layout,booktext(),booklength(),&ui.options,measure,NULL)){
  snprintf(status,sizeof(status),"Book layout exceeds available memory.");return false;
 }
 sync_position();return true;
}
#ifndef FOLIO_HOST_PREVIEW
/* Present a real frame BEFORE synchronous SD/parser work. These stages are
   deliberately not a fake percentage or a promise of cancellable SD calls. */
static void busy_frame(const char *message){
 if(!aptIsActive())return;
 if(!C3D_FrameBegin(C3D_FRAME_SYNCDRAW))return;
 const Palette *p=&palettes[ui.options.theme];C2D_TextBufClear(textBuf);
 C2D_TargetClear(top,color(p->paper));C2D_SceneBegin(top);
 label(20,72,360,22,p->ink,"Folio",true);
 label(12,112,376,13,p->ink,message,true);
 label(12,143,376,10,p->muted,tr("Large books and slow SD cards may take longer.","Velké knihy a pomalé SD karty mohou trvat déle."),true);
 C3D_FrameEnd(0);
}
static unsigned hash_path(const char *s){unsigned h=2166136261u;while(*s){h^=(unsigned char)*s++;h*=16777619u;}return h;}
static void save_progress(void){
 if(settings_job_busy())return; /* Never queue another SD call behind a stalled save. */
 if(!dirty||!currentPath[0])return;
 busy_frame(tr("Saving reading position…","Ukládám pozici čtení…"));
 char p[FOLIO_PATH_MAX];snprintf(p,sizeof(p),PROGRESS_DIR "/%08x.state",hash_path(currentPath));
 FILE *f=fopen(p,"w");if(!f){saveFailed=true;return;}
 int result=fprintf(f,"%u\n%u\n%d\n%u\n%llu\n",(unsigned)position,(unsigned)bookmark,ui.hasBookmark?1:0,(unsigned)doc.length,(unsigned long long)currentLastRead);
 int closed=fclose(f);saveFailed=result<0||closed!=0;if(!saveFailed)dirty=false;
}
static void load_progress(void){
 char p[FOLIO_PATH_MAX];snprintf(p,sizeof(p),PROGRESS_DIR "/%08x.state",hash_path(currentPath));
 position=bookmark=0;ui.hasBookmark=false;unsigned a,b;int c;
 FILE *f=fopen(p,"r");if(f){if(fscanf(f,"%u%u%d",&a,&b,&c)==3){position=a;bookmark=b;ui.hasBookmark=c!=0&&b<doc.length;}fclose(f);}
 if(position>=doc.length)position=0;
 dirty=false;saveFailed=false;
}
static void move_to(size_t offset){
 if(!doc.text)return;
 position=offset<doc.length?offset:doc.length?doc.length-1:0;
 ui.focusLine=0;dirty=true;sync_position();
}
static void move_lines(int delta){
 if(!doc.text||!layout.count)return;
 if(delta>=layout.rows&&firstLine+(size_t)layout.rows>=layout.count)return;
 long next=(long)firstLine+delta;if(next<0)next=0;if((size_t)next>=layout.count)next=layout.count-1;
 int activeLine=ui.focusLine;move_to(layout.lines[next].start);
 if(delta==1||delta==-1)ui.focusLine=reader_focus_move(activeLine,0,reader_visible(&layout,firstLine));
}
static void mark_read(void){
 uint64_t now=osGetTime();currentLastRead=now>latestRead?now:latestRead+1;
 latestRead=currentLastRead;dirty=true;
}
static void entry_progress(Entry *e){
 e->percent=-1;e->lastRead=0;if(e->directory)return;
 if(doc.text&&!strcmp(e->path,currentPath)){e->percent=doc.length?(int)(position*100/doc.length):0;e->lastRead=currentLastRead;return;}
 char p[FOLIO_PATH_MAX];snprintf(p,sizeof(p),PROGRESS_DIR "/%08x.state",hash_path(e->path));
 FILE *f=fopen(p,"r");if(!f)return;
 unsigned offset=0,mark=0,length=0;int has=0;
 unsigned long long recent=0;
 int n=fscanf(f,"%u%u%d%u%llu",&offset,&mark,&has,&length,&recent);fclose(f);
 if(n==5){e->lastRead=recent;if(e->lastRead>latestRead)latestRead=e->lastRead;}
 if(n>=4&&length&&offset<=length)e->percent=(int)((uint64_t)offset*100/length);
 else if(n>=3)e->percent=-2; /* Older state: position known, text length unknown. */
}
static void filter_entries(void){
 qsort(entries,entryCount,sizeof(entries[0]),compare_entries);
 ui.listCount=0;for(int i=0;i<entryCount;i++){
  FolioFormat f=folio_detect_format(entries[i].name);
  bool keep=ui.view==UI_FOLDER?entries[i].directory:!entries[i].directory&&
   (!ui.filter||(ui.filter==1&&f==FOLIO_FORMAT_EPUB)||(ui.filter==2&&f==FOLIO_FORMAT_TEXT)||(ui.filter==3&&f==FOLIO_FORMAT_HTML));
  if(keep)filtered[ui.listCount++]=i;
 }
 ui.selected=ui.listPage=0;
}
static void scan_folder(void){
 if(settings_job_busy()){snprintf(status,sizeof(status),"Settings I/O pending; please wait.");return;}
 busy_frame(tr("Reading books folder…","Načítám složku knih…"));
 entryCount=0;DIR *d=opendir(folder);
 if(!d){ui.libraryReady=false;snprintf(status,sizeof(status),"Cannot open SD folder.");filter_entries();return;}
 ui.libraryReady=true;struct dirent *e;bool truncated=false;
 while((e=readdir(d))){
  if(e->d_name[0]=='.')continue;
  char path[FOLIO_PATH_MAX];if(strlen(folder)+strlen(e->d_name)+1>=sizeof(path))continue;
  size_t prefix=strlen(folder);memcpy(path,folder,prefix);strcpy(path+prefix,e->d_name);
  struct stat st;if(stat(path,&st))continue;
  bool directory=S_ISDIR(st.st_mode);FolioFormat f=folio_detect_format(e->d_name);
  if(!directory&&(f==FOLIO_FORMAT_UNKNOWN||f==FOLIO_FORMAT_PDF))continue;
  if(entryCount==256){truncated=true;break;}
  Entry *x=&entries[entryCount++];snprintf(x->path,sizeof(x->path),"%s",path);
  snprintf(x->name,sizeof(x->name),"%.123s%s",e->d_name,directory?"/":"");x->directory=directory;entry_progress(x);
 }
 closedir(d);filter_entries();
 snprintf(status,sizeof(status),truncated?"First 256 entries only; split books into folders.":"%s",folder);
}
static void open_entry(int index){
 if(settings_job_busy()){snprintf(status,sizeof(status),"Settings I/O pending; please wait.");return;}
 if(index<0||index>=ui.listCount)return;
 Entry *x=&entries[filtered[index]];
 if(x->directory){if(strlen(x->path)+2>sizeof(folder))return;snprintf(folder,sizeof(folder),"%s/",x->path);scan_folder();return;}
 if(doc.text&&!strcmp(x->path,currentPath)){mark_read();save_progress();ui.view=UI_HOME;return;}
 FolioDocument next;char error[160];
 busy_frame(tr("Opening and decoding book…","Otevírám a dekóduji knihu…"));
 if(!folio_load_document(x->path,&next,error,sizeof(error))){snprintf(status,sizeof(status),"%s",error);return;}
 ReaderLayout nextLayout={0};
 busy_frame(tr("Preparing pages…","Připravuji stránky…"));
 if(!reader_layout(&nextLayout,next.text,next.length,&ui.options,measure,NULL)){folio_free_document(&next);snprintf(status,sizeof(status),"Book layout exceeds available memory.");return;}
 save_progress();folio_free_document(&doc);reader_layout_free(&layout);doc=next;layout=nextLayout;
 snprintf(currentPath,sizeof(currentPath),"%s",x->path);load_progress();sync_position();ui.focusLine=0;ui.view=UI_HOME;
 mark_read();save_progress();
}
static void parent_folder(void){
 size_t n=strlen(folder);if(n<=6)return;
 if(folder[n-1]=='/')folder[--n]=0;
 char *slash=strrchr(folder,'/');if(slash)slash[1]=0;scan_folder();
}
static void save_preferences(void){
 preferences.language=ui.language;
 preferencesQueued=true;configFailed=false;
 /* Capture/queue only. The first save attempt starts after the next frame. */
}
static void act(UiAction action,int value){
 if(settings_job_busy()&&(action==ACT_CHOOSE_FOLDER||action==ACT_USE_FOLDER||action==ACT_PARENT||action==ACT_SCAN||action==ACT_ENTRY)){
  snprintf(status,sizeof(status),"Settings I/O pending; please wait.");return;
 }
 UiView oldView=ui.view;
 ReaderOptions before=ui.options;
 bool relayout=ui_change(&ui,action,value);
 if(relayout){if(doc.text)busy_frame(tr("Updating page layout…","Upravuji rozložení stránek…"));if(!reflow())ui.options=before;}
 switch(action){
 case ACT_RESUME:if(doc.text){mark_read();save_progress();}break;
 case ACT_LIBRARY:
  save_progress();
  for(int i=0;i<entryCount;i++)if(!strcmp(entries[i].path,currentPath))entry_progress(&entries[i]);
  if(settings_job_busy()){filter_entries();break;}
  if(preferences.folder[0]&&strcmp(folder,preferences.folder)){snprintf(folder,sizeof(folder),"%s",preferences.folder);scan_folder();break;}
  filter_entries();break;
 case ACT_FILTER:filter_entries();break;
 case ACT_SCAN:scan_folder();break;
 case ACT_PARENT:parent_folder();break;
 case ACT_ENTRY:open_entry(value);break;
 case ACT_CHOOSE_FOLDER:
  if(preferences.folder[0])snprintf(folder,sizeof(folder),"%s",preferences.folder);
  else snprintf(folder,sizeof(folder),"sdmc:/");
  scan_folder();break;
 case ACT_USE_FOLDER:
  if(ui.libraryReady){snprintf(preferences.folder,sizeof(preferences.folder),"%s",folder);ui.view=UI_LIBRARY;filter_entries();save_preferences();}
  break;
 case ACT_BACK:
  if(oldView==UI_FOLDER){snprintf(folder,sizeof(folder),"%s",preferences.folder[0]?preferences.folder:"sdmc:/");scan_folder();}
  break;
 case ACT_LANGUAGE:save_preferences();if(!doc.text){position=0;reflow();}break;
 case ACT_FOCUS_MOVE:ui.focusLine=reader_focus_move(ui.focusLine,value,reader_visible(&layout,firstLine));break;
 case ACT_BOOKMARK:if(doc.text){
  if(ui.bookmarkHere)ui.hasBookmark=false;
  else{bookmark=position;ui.hasBookmark=true;}
  dirty=true;sync_position();save_progress();
 }break;
 case ACT_BOOKMARK_GO:if(ui.hasBookmark){move_to(bookmark);ui.view=UI_HOME;}break;
 case ACT_JUMP_CONFIRM:
  if(doc.text){if(ui.jumpMode){size_t line=(size_t)(ui.jumpValue-1)*layout.rows;if(line<layout.count)move_to(layout.lines[line].start);}
   else move_to(doc.chapterCount?doc.chapters[ui.jumpValue-1].offset:0);
   ui.view=UI_HOME;}break;
 case ACT_PAGE_INPUT:{
  SwkbdState keyboard;char number[16];snprintf(number,sizeof(number),"%d",ui.jumpValue);
  swkbdInit(&keyboard,SWKBD_TYPE_NUMPAD,2,8);swkbdSetInitialText(&keyboard,number);swkbdSetHintText(&keyboard,tr("Page number","Číslo stránky"));
  swkbdSetButton(&keyboard,SWKBD_BUTTON_LEFT,tr("Cancel","Zrušit"),false);
  swkbdSetButton(&keyboard,SWKBD_BUTTON_RIGHT,tr("Go","Přejít"),true);
  if(swkbdInputText(&keyboard,number,sizeof(number))==SWKBD_BUTTON_CONFIRM){long page=strtol(number,NULL,10);ui.jumpValue=page<1?1:page>ui.pageCount?ui.pageCount:(int)page;}
  lastInput=osGetTime();break;}
 default:break;
 }
 focused=-1;
}
#endif
/* Draw every row independently. Focus opacity is never applied to a whole text block. */
static void draw_line(const ReaderLine *line,float y,unsigned ink){
 const char *s=booktext();size_t p=line->start;float x=22;
 C2D_Font font=fonts[ui.options.font];int wordIndex=0,boldCount=0;bool inWord=false;
 while(p<line->end){size_t start=p;uint32_t cp=reader_codepoint(s,line->end,&p);
  if(cp=='\r'||cp=='\n')continue;
  bool word=reader_word_char(cp);
  if(word&&!inWord){size_t scan=start;int chars=0;while(scan<booklength()){uint32_t next=reader_codepoint(s,booklength(),&scan);if(!reader_word_char(next))break;++chars;}
   boldCount=reader_bold_prefix(chars,ui.options.strength);wordIndex=0;
  }
  inWord=word;char glyph[8];size_t n=p-start;if(n>4)n=4;memcpy(glyph,s+start,n);glyph[n]=0;if(cp=='\t')strcpy(glyph," ");
  text(font,x,y,ui.options.size,ink,glyph);
  if(ui.options.bionic&&word&&wordIndex<boldCount)text(font,x+.95f,y,ui.options.size,ink,glyph);
  if(word)++wordIndex;
  x+=fmaxf(.5f,advance(font,cp,ui.options.size)+ui.options.size*ui.options.tracking/100.f);
 }
}
static const char *chapter_title(int index){
 if(!doc.chapterCount)return tr("Beginning of book","Začátek knihy");
 if(index==0&&doc.format!=FOLIO_FORMAT_EPUB&&!strcmp(doc.chapters[0].title,"Start"))return tr("Beginning of book","Začátek knihy");
 return doc.chapters[index].title;
}
static void draw_reader(void){
 const Palette *p=&palettes[ui.options.theme];rect(0,0,400,240,p->paper);
 label(48,11,304,10,p->muted,doc.text?doc.title:"Folio",true);
 const char *format=!doc.text?"FOLIO":doc.format==FOLIO_FORMAT_EPUB?"EPUB":doc.format==FOLIO_FORMAT_HTML?"HTML":"TXT";
 roundrect(355,9,36,17,5,blend(p->ink,p->paper,.07f));label(356,12,34,8,p->muted,format,true);
 int visible=reader_visible(&layout,firstLine);
 for(int row=0;row<visible;row++){
  unsigned ink=ui.options.focus&&row!=ui.focusLine?blend(p->ink,p->paper,.27f):p->ink;
  draw_line(&layout.lines[firstLine+row],36+row*layout.height,ink);
 }
 char foot[96];snprintf(foot,sizeof(foot),"%s",doc.chapterCount?chapter_title(ui.currentChapter):doc.text?tr("Reading","Čtení"):tr("Welcome to your library","Vítej ve své knihovně"));
 label(22,223,265,9,p->muted,foot,false);
 snprintf(foot,sizeof(foot),"%d / %d · %u%%",ui.currentPage,ui.pageCount,doc.length?(unsigned)(position*100/doc.length):0);
 label(284,223,94,9,p->muted,foot,true);
 static const u8 shade[]={148,102,56,26,0};
 C2D_DrawRectSolid(0,0,0,400,240,C2D_Color32(0,0,0,shade[ui.options.brightness-1]));
}
static const char *progress_caption(const Entry *entry,char *out,size_t n){
 if(entry->directory)return tr("Folder","Složka");
 if(entry->percent==-2)return tr("Position saved","Pozice uložena");
 if(entry->percent<0)return tr("New book","Nová kniha");
 snprintf(out,n,"%d%%",entry->percent);return out;
}
static void cover_title(float x,float y,float w,const char *name,unsigned ink){
 char title[128];snprintf(title,sizeof(title),"%s",name);char *dot=strrchr(title,'.');if(dot)*dot=0;
 size_t n=strlen(title),at=0;
 for(int row=0;row<5&&at<n;row++){
  size_t start=at,end=at,lastSpace=at;float used=0;
  while(at<n){size_t before=at;uint32_t cp=reader_codepoint(title,n,&at);float step=advance(fonts[0],cp,10);
   if(used+step>w&&before>start){at=before;break;}used+=step;end=at;if(cp==' ')lastSpace=before;
  }
  if(at<n&&lastSpace>start){end=lastSpace;at=lastSpace+1;}
  if(end==start)end=at;
  char line[128];size_t take=end-start;memcpy(line,title+start,take);line[take]=0;
  label(x,y+row*13,w,10,ink,line,false);
 }
}
static void draw_library_top(void){
 const Palette *p=&palettes[ui.options.theme];rect(0,0,400,240,p->paper);
 label(14,10,280,14,p->ink,ui.view==UI_FOLDER?tr("Choose books folder","Vyber složku knih"):tr("Your library","Tvoje knihovna"),false);
 char value[96];snprintf(value,sizeof(value),"%d / %d",ui.listCount?ui.selected+1:0,ui.listCount);
 label(305,13,80,10,p->muted,value,true);
 if(!ui.listCount){
  label(20,91,360,13,p->ink,ui.view==UI_FOLDER?tr("No subfolders","Žádné podsložky"):tr("A home for your books","Domov pro tvoje knihy"),true);
  label(20,117,360,10,p->muted,ui.view==UI_FOLDER?tr("Use this folder or go to its parent.","Použij tuto složku nebo jdi o úroveň výš."):tr("Choose a folder on the touchscreen below.","Vyber složku na dotykovém displeji dole."),true);
 }else for(int slot=0;slot<4;slot++){
  int index=ui.listPage*4+slot;if(index>=ui.listCount)break;Entry *entry=&entries[filtered[index]];
  int x=12+slot*97;bool active=index==ui.selected;
  unsigned cover=blend(p->ink,p->paper,.08f+(slot%3)*.035f);
  roundrect(x,39,86,141,5,active?p->ink:blend(p->ink,p->paper,.2f));
  roundrect(x+2,41,82,137,4,cover);rect(x+6,45,3,128,blend(p->ink,cover,.2f));
  const char *format=entry->directory?tr("FOLDER","SLOŽKA"):folio_detect_format(entry->name)==FOLIO_FORMAT_EPUB?"EPUB":folio_detect_format(entry->name)==FOLIO_FORMAT_HTML?"HTML":"TXT";
  label(x+14,51,65,8,p->muted,format,false);
  cover_title(x+14,77,65,entry->name,p->ink);
  label(x+5,156,76,9,p->ink,progress_caption(entry,value,sizeof(value)),true);
  if(!entry->directory&&entry->percent>=0){rect(x+7,170,72,2,blend(p->ink,cover,.15f));rect(x+7,170,72*entry->percent/100.f,2,p->ink);}
 }
 if(ui.listCount)label(14,191,372,11,p->ink,entries[filtered[ui.selected]].name,true);
 label(12,218,376,9,p->muted,ui.view==UI_FOLDER?
  tr("A: open folder   B: parent   Y: use folder","A: otevřít   B: o úroveň výš   Y: použít složku"):
  tr("Left/right: books   Touch: filters   A: open","Vlevo/vpravo: knihy   Filtry: dotykem   A: otevřít"),true);
}
static void draw_bottom(void){
 if(ui.sleeping)return;
 const Palette *p=&palettes[ui.options.theme];rect(0,0,320,240,p->bg);
 const char *titles[]={doc.text?doc.title:"Folio",tr("Display","Displej"),tr("Type","Písmo"),tr("Settings","Nastavení"),tr("Library","Knihovna"),tr("Go to…","Přejít na…"),tr("Folder","Složka")};
 label(ui.view==UI_HOME?12:48,15,(ui.view==UI_LIBRARY||ui.view==UI_FOLDER)?170:ui.view==UI_HOME?258:252,12,p->fg,titles[ui.view],false);
 if(ui.view==UI_FONT)label(10,178,300,8,p->secondary,"Folio 0.4.6 · Faefolk333",true);
 if(ui.view<=UI_SETTINGS){
  roundrect(110,200,200,30,8,p->card);
  for(int i=0;i<5;i++)roundrect(158+i*20,211,14,8,2,i<ui.options.brightness?p->accent:p->border);
 }
 char value[128];
 if(ui.view==UI_DISPLAY)label(18,43,280,9,p->secondary,tr("Reading and touchscreen colours","Barvy čtecího i dotykového displeje"),false);
 if(ui.view==UI_FONT){
  snprintf(value,sizeof(value),tr("Size %d px","Velikost %d px"),ui.options.size);label(58,88,204,13,p->fg,value,true);
  label(10,117,146,9,p->secondary,tr("Line spacing","Řádkování"),false);label(164,117,146,9,p->secondary,tr("Letter spacing","Proklad písmen"),false);
  snprintf(value,sizeof(value),"%.2f",ui.options.leading/100.f);label(42,144,82,10,p->fg,value,true);
  snprintf(value,sizeof(value),"%+.2f em",ui.options.tracking/100.f);label(196,144,82,10,p->fg,value,true);
 }
 if(ui.view==UI_SETTINGS){
  roundrect(10,44,148,91,9,p->card);roundrect(166,44,144,91,9,p->card);
  roundrect(10,143,148,50,9,p->card);roundrect(166,143,144,50,9,p->card);
  label(18,49,132,10,p->fg,tr("Reading style","Způsob čtení"),false);label(174,49,126,10,p->fg,tr("Active line","Aktivní řádek"),false);
  snprintf(value,sizeof(value),tr("Line %d / %d","Řádek %d / %d"),ui.focusLine+1,reader_visible(&layout,firstLine));label(174,109,126,9,p->secondary,value,true);
  label(18,149,132,10,p->fg,tr("Language","Jazyk"),false);
  label(174,146,124,9,p->secondary,tr("Lower display","Spodní displej"),false);
 }
 if(ui.view==UI_LIBRARY||ui.view==UI_FOLDER){
  label(10,190,300,8,p->secondary,i18n_error(status),false);
  if(ui.view==UI_FOLDER)label(12,47,296,9,p->secondary,folder,false);
  if(!ui.listCount){label(18,105,284,11,p->fg,ui.view==UI_FOLDER?tr("No subfolders","Žádné podsložky"):ui.libraryReady?tr("No books for this filter","Žádné knihy pro tento filtr"):tr("Your books on the SD card","Tvoje knihy na SD kartě"),true);
   label(18,131,284,9,p->secondary,tr("TXT · HTML · DRM-free EPUB","TXT · HTML · EPUB bez DRM"),true);}
 }
 if(ui.view==UI_JUMP){
  if(!ui.jumpMode){snprintf(value,sizeof(value),tr("Chapter %d / %d","Kapitola %d / %d"),ui.jumpValue,ui.chapterCount);label(57,95,206,12,p->fg,value,true);}
  const char *chapter=chapter_title(ui.jumpMode?ui.currentChapter:ui.jumpValue-1);
  label(12,126,296,9,p->secondary,chapter,true);
  if(!ui.jumpMode)label(12,181,296,8,p->secondary,tr("Choose a chapter and press Go","Vyber kapitolu a potvrď tlačítkem Přejít"),true);
 }
 for(int i=0;i<controls.count;i++){
  UiControl *c=&controls.controls[i];unsigned bg=c->active?p->accent:p->card,fg=c->active?p->onAccent:p->fg;
  if(!c->enabled)fg=blend(fg,bg,.38f);
  roundrect(c->x,c->y,c->w,c->h,7,i==focused?p->accent:p->border);
  roundrect(c->x+1,c->y+1,c->w-2,c->h-2,6,bg);
  if(c->style==3){const Palette *sample=&palettes[c->value];roundrect(c->x+5,c->y+5,c->w-10,24,4,sample->paper);
   label(c->x+6,c->y+8,c->w-12,13,sample->ink,"Aa",true);label(c->x+3,c->y+34,c->w-6,9,fg,c->label,true);
  }else if(c->style==1){
   if(c->action==ACT_FONT)text(fonts[0],c->x+12,c->y+7,17,fg,"Aa");
   else {roundrect(c->x+12,c->y+9,21,15,3,fg);roundrect(c->x+14,c->y+11,17,11,2,bg);}
   label(c->x+12,c->y+30,c->w-24,12,fg,c->label,false);label(c->x+12,c->y+48,c->w-24,8,p->secondary,c->sub,false);
  }else if(c->style==2){label(c->x+12,c->y+6,c->w-24,11,fg,c->label,false);label(c->x+12,c->y+23,c->w-24,8,p->secondary,c->sub,false);
  }else{
   const char *caption=c->label;
   if(c->action==ACT_ENTRY){Entry *entry=&entries[filtered[c->value]];snprintf(value,sizeof(value),"%s%s",entry->directory?"> ":"",entry->name);caption=value;}
   if(c->action==ACT_PAGE_INPUT){snprintf(value,sizeof(value),tr("%d / %d  (enter)","%d / %d  (zadat)"),ui.jumpValue,ui.pageCount);caption=value;}
   label(c->x+5,c->y+(c->h-10)/2,c->w-10,10,fg,caption,c->action!=ACT_ENTRY);
  }
 }
 if((ui.view==UI_LIBRARY||ui.view==UI_FOLDER)&&preferencesSaving){
  rect(8,189,304,10,p->bg);
  snprintf(value,sizeof(value),tr("Saving settings: step %u. START: exit","Ukládání: krok %u. START: konec"),preferenceStage);
  label(12,190,296,8,p->fg,value,true);
 }else if((ui.view==UI_LIBRARY||ui.view==UI_FOLDER)&&configFailed){
  rect(8,189,304,10,p->bg);
  snprintf(value,sizeof(value),tr("Settings not saved (step %u, %08lX)","Nastavení neuloženo (krok %u, %08lX)"),preferenceStage,(unsigned long)(uint32_t)preferenceError);
  label(12,190,296,8,p->fg,value,true);
 }else if((ui.view==UI_LIBRARY||ui.view==UI_HOME)&&saveFailed){roundrect(8,176,304,19,4,p->accent);label(12,180,296,8,p->onAccent,
  configFailed?tr("Settings not saved. Check SD:/Folio","Nastavení neuloženo. Zkontroluj SD:/Folio"):tr("Not saved: check SD:/Folio/progress","Neuloženo: zkontroluj SD:/Folio/progress"),true);}
}
#ifndef FOLIO_HOST_PREVIEW
static aptHookCookie lifecycleHook;
static bool lcdNeedsApply;
static void on_lifecycle(APT_HookType event,void *unused){
 (void)unused;
 if(event==APTHOOK_ONSUSPEND||event==APTHOOK_ONSLEEP||event==APTHOOK_ONEXIT)backlight_restore();
 if(event==APTHOOK_ONRESTORE||event==APTHOOK_ONWAKEUP){backlight_resume();lcdNeedsApply=true;lastInput=osGetTime();}
}
static void focus_control(int delta){
 if(!controls.count)return;
 for(int i=0;i<controls.count;i++){focused=(focused+delta+controls.count)%controls.count;if(controls.controls[focused].enabled)break;}
}
int main(int argc,char **argv){
 (void)argc;(void)argv;bool c3dReady=false,c2dReady=false,hooked=false,bootPending=true,skipScan=false;
 bool appliedSleep=false;const char *failure=NULL;
 ui.options=reader_defaults();ui.view=UI_LIBRARY;i18n_set(LANG_EN);startup_begin();
 startup_stage("00a Checking HID memory");if(!hidSharedMem)goto cleanup;
 startup_stage("00b hidScanInput BEGIN");hidScanInput();startup_stage("00c hidScanInput OK");if(hidKeysHeld()&KEY_START)goto cleanup;
 skipScan=(hidKeysHeld()&KEY_Y)!=0;
 startup_stage("01 No startup SD writes");startup_stage("02 System font");
 if(R_FAILED(fontEnsureMapped())){failure="System font unavailable";goto init_failed;}
 startup_stage("03 C3D_Init");if(!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)){failure="C3D_Init failed";goto init_failed;}c3dReady=true;
 startup_stage("04 C2D_Init");if(!C2D_Init(C2D_DEFAULT_MAX_OBJECTS)){failure="C2D_Init failed";goto init_failed;}c2dReady=true;C2D_Prepare();
 startup_stage("05 Screen targets");top=C2D_CreateScreenTarget(GFX_TOP,GFX_LEFT);bottom=C2D_CreateScreenTarget(GFX_BOTTOM,GFX_LEFT);
 if(!top||!bottom){failure="Screen allocation failed";goto init_failed;}
 startup_stage("06 Text buffer");textBuf=C2D_TextBufNew(8192);if(!textBuf){failure="Text allocation failed";goto init_failed;}
 startup_stage("07 Embedded fonts (no SD access)");
 fonts[0]=C2D_FontLoadFromMem(folio_sans_start,folio_sans_end-folio_sans_start);
 fonts[1]=C2D_FontLoadFromMem(folio_serif_start,folio_serif_end-folio_serif_start);
 fonts[3]=C2D_FontLoadFromMem(folio_inter_start,folio_inter_end-folio_inter_start);
 fonts[4]=C2D_FontLoadFromMem(folio_garamond_start,folio_garamond_end-folio_garamond_start);
 fonts[5]=C2D_FontLoadFromMem(folio_baskerville_start,folio_baskerville_end-folio_baskerville_start);
 for(int i=0;i<READER_FONT_COUNT;i++)if(i!=2&&!fonts[i]){failure="Embedded font allocation failed";goto init_failed;}
 if(!reflow()){failure="Initial layout allocation failed";goto init_failed;}
 startup_stage("08 APT event loop");if(!aptMainLoop())goto cleanup;
 startup_stage("09 APT returned; GPU handover");startup_use_gpu();lastInput=osGetTime();
 hidSetRepeatParameters(24,6);
 /* HBL gets an exit instruction; installed CIA titles use normal HOME. */
 aptSetHomeAllowed(!envIsHomebrew());
 u64 homeNoticeUntil=0;
 aptHook(&lifecycleHook,on_lifecycle,NULL);hooked=true;
 while(aptMainLoop()){
  hidScanInput();u32 down=hidKeysDown(),held=hidKeysHeld(),nav=hidKeysDownRepeat();u64 now=osGetTime();
  if(aptCheckHomePressRejected())homeNoticeUntil=now+6000;
  if(down&KEY_START){busy_frame(tr("Closing Folio…","Ukončuji Folio…"));break;}
  FolioSettings savedPreferences;
  int completed=settings_job_poll(&savedPreferences);
  preferencesSaving=settings_job_busy();
  if(completed){
   preferenceStage=settings_job_stage();
   configFailed=completed<0;preferences.generation=completed>0?savedPreferences.generation:preferences.generation;
   if(completed>0&&!preferences.folder[0])snprintf(preferences.folder,sizeof(preferences.folder),"%s",savedPreferences.folder);
   preferenceError=settings_job_error();
   if(completed<0)snprintf(status,sizeof(status),"Settings could not be saved. Folder works for this session.");
   else snprintf(status,sizeof(status),"%.159s",folder);
  }
  if(preferencesSaving){
   preferenceStage=settings_job_stage();
   if(now-preferencesStarted>3000)snprintf(status,sizeof(status),"Settings I/O is slow. START exits without waiting.");
  }
  bool inLibrary=ui.view==UI_LIBRARY||ui.view==UI_FOLDER;
  /* Reading controls neither wake the lower display nor postpone its timer. */
  const u32 wakeMask=KEY_TOUCH|KEY_SELECT;
  const u32 uiMask=wakeMask|KEY_A|KEY_B|(ui.view==UI_HOME?0:KEY_DUP|KEY_DDOWN|KEY_DLEFT|KEY_DRIGHT);
  if(((down|held)&uiMask)||(inLibrary&&((down|held)&(KEY_CPAD_UP|KEY_CPAD_DOWN|KEY_CPAD_LEFT|KEY_CPAD_RIGHT))))lastInput=now;
  if(!preferencesSaving&&!inLibrary&&ui.hasBook&&ui.options.sleepSeconds&&now-lastInput>(u64)ui.options.sleepSeconds*1000)ui.sleeping=true;
  bool woke=ui_wake(&ui,(down&wakeMask)!=0);if(woke)lastInput=now;
  ui_build(&ui,&controls);
  if(!woke&&!ui.sleeping){
   if(down&KEY_TOUCH){touchPosition touch;hidTouchRead(&touch);int hit=ui_hit(&controls,touch.px,touch.py);if(hit>=0)act(controls.controls[hit].action,controls.controls[hit].value);}
   if(down&KEY_B)act(ui.view==UI_FOLDER?ACT_PARENT:ACT_BACK,0);
   if(inLibrary){
    if(ui.view==UI_LIBRARY){
     if(nav&(KEY_DLEFT|KEY_CPAD_LEFT|KEY_DUP|KEY_CPAD_UP))ui_move_entry(&ui,-1);
     if(nav&(KEY_DRIGHT|KEY_CPAD_RIGHT|KEY_DDOWN|KEY_CPAD_DOWN))ui_move_entry(&ui,1);
    }else{
     if(nav&(KEY_DUP|KEY_CPAD_UP))ui_move_entry(&ui,-1);
     if(nav&(KEY_DDOWN|KEY_CPAD_DOWN))ui_move_entry(&ui,1);
     if(down&(KEY_DLEFT|KEY_CPAD_LEFT))act(ACT_PARENT,0);
     if(down&(KEY_DRIGHT|KEY_CPAD_RIGHT))act(ACT_ENTRY,ui.selected);
    }
    if(down&KEY_L)ui_move_entry(&ui,-4);
    if(down&KEY_R)ui_move_entry(&ui,4);
    if(down&KEY_A)act(ACT_ENTRY,ui.selected);
    if(down&KEY_X)act(ACT_SETTINGS,0);
    if(down&KEY_Y)act(ui.view==UI_FOLDER?ACT_USE_FOLDER:ACT_CHOOSE_FOLDER,0);
   }else{
    if(nav&(KEY_DLEFT|(ui.view==UI_HOME?0:KEY_DUP)))focus_control(-1);
    if(nav&(KEY_DRIGHT|(ui.view==UI_HOME?0:KEY_DDOWN)))focus_control(1);
    if(down&KEY_A){if(focused<0)focus_control(1);else if(focused<controls.count&&controls.controls[focused].enabled)act(controls.controls[focused].action,controls.controls[focused].value);}
   }
  }
  /* Keep the upper reader interactive even while the touchscreen is dark. */
  if(!woke&&!inLibrary&&ui.view!=UI_LIBRARY&&ui.view!=UI_FOLDER){
   if(down&(KEY_L|KEY_ZL|KEY_CPAD_LEFT))move_lines(-layout.rows);
   if(down&(KEY_R|KEY_ZR|KEY_CPAD_RIGHT))move_lines(layout.rows);
   u32 scrollUp=KEY_CPAD_UP|(ui.view==UI_HOME?KEY_DUP:0);
   u32 scrollDown=KEY_CPAD_DOWN|(ui.view==UI_HOME?KEY_DDOWN:0);
   if(nav&(scrollUp|scrollDown))move_lines(nav&scrollUp?-1:1);
   if(down&KEY_X)act(ACT_BOOKMARK,0);
  }
  if(appliedSleep!=ui.sleeping||lcdNeedsApply){
   lcdFailed=!backlight_sleep(ui.sleeping);appliedSleep=ui.sleeping;lcdNeedsApply=false;
  }
  ui_build(&ui,&controls);
  if(!C3D_FrameBegin(C3D_FRAME_SYNCDRAW))continue;
  C2D_TextBufClear(textBuf);C2D_TargetClear(top,color(palettes[ui.options.theme].paper));C2D_TargetClear(bottom,C2D_Color32(0,0,0,255));
  C2D_SceneBegin(top);
  if(ui.view==UI_LIBRARY||ui.view==UI_FOLDER||(ui.view==UI_SETTINGS&&(ui.settingsReturn==UI_LIBRARY||ui.settingsReturn==UI_FOLDER)))draw_library_top();
  else draw_reader();
  if(now<homeNoticeUntil){rect(0,198,400,42,palettes[ui.options.theme].paper);
   label(6,204,388,12,palettes[ui.options.theme].ink,tr("Press START to exit to Homebrew Launcher","Pro návrat do Homebrew Launcheru stiskni START"),true);}
  if(lcdFailed){rect(0,224,400,16,palettes[ui.options.theme].paper);label(6,226,388,8,palettes[ui.options.theme].ink,
   tr("Backlight unavailable: software blanking only","Podsvícení nelze vypnout: pouze černý obraz"),true);}
  C2D_SceneBegin(bottom);draw_bottom();C3D_FrameEnd(0);
  if(bootPending){
   bootPending=false;
   if(skipScan)snprintf(status,sizeof(status),"Safe start: automatic scan skipped.");
   else{
    busy_frame(tr("Loading preferences…","Načítám nastavení…"));
    settings_load("sdmc:/Folio",&preferences);ui.language=preferences.language;i18n_set(ui.language);reflow();
    if(preferences.folder[0]){snprintf(folder,sizeof(folder),"%s",preferences.folder);scan_folder();}
   }
   lastInput=osGetTime();
  }
  if(preferencesQueued&&!settings_job_busy()){
   preferencesQueued=false;preferencesStarted=osGetTime();
   if(!settings_job_start(&preferences)){configFailed=true;preferenceStage=0;preferenceError=-1;}
   else preferencesSaving=true;
  }
 }
 save_progress();busy_frame(envIsHomebrew()?tr("Returning to Homebrew Launcher…","Vracím se do Homebrew Launcheru…"):tr("Closing Folio…","Ukončuji Folio…"));backlight_restore();goto cleanup;
init_failed:startup_error(failure);
cleanup:
 settings_job_shutdown();
 if(hooked)aptUnhook(&lifecycleHook);
 backlight_exit();
 reader_layout_free(&layout);folio_free_document(&doc);if(textBuf)C2D_TextBufDelete(textBuf);
 for(int i=0;i<READER_FONT_COUNT;i++)if(fonts[i])C2D_FontFree(fonts[i]);
 if(c2dReady)C2D_Fini();
 if(c3dReady)C3D_Fini();
 gfxExit();return 0;
}
#endif
