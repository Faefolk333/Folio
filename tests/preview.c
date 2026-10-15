#define FOLIO_HOST_PREVIEW
#include "../source/main.c"
int main(int argc,char **argv){
 fonts[0]=preview_font("assets/fonts/sans.bcfnt",0);fonts[1]=preview_font("assets/fonts/serif.bcfnt",1);fonts[2]=fonts[0];
 fonts[3]=preview_font("assets/fonts/inter.bcfnt",3);fonts[4]=preview_font("assets/fonts/ebgaramond.bcfnt",4);fonts[5]=preview_font("assets/fonts/librebaskerville.bcfnt",5);
 ui.options=reader_defaults();ui.options.brightness=5;ui.options.focus=argc>3?atoi(argv[3]):0;ui.focusLine=2;
 ui.view=argc>1?atoi(argv[1]):0;ui.options.theme=argc>2?atoi(argv[2]):1;
 ui.language=argc>4?atoi(argv[4]):0;i18n_set(ui.language);
 doc.text="Za hranicí každodenního hluku\n\nKniha je tiché místo, do kterého se můžeme vracet. Stačí otočit stránku a na chvíli zpomalit.\n\nPříběh pokračuje právě tam, kde jsme jej včera opustili.";
 doc.length=strlen(doc.text);strcpy(doc.title,"Klidné čtení");doc.chapterCount=1;strcpy(doc.chapters[0].title,"První kapitola");
 if(argc>5){
  int sample=atoi(argv[5]);
  const char *titles[]={"Alice's Adventures in Wonderland","Pride and Prejudice","Frankenstein","Máj — Karel Hynek Mácha"};
  const char *chapters[]={"Down the Rabbit-Hole","Chapter 1","Letter 1","První zpěv"};
  const char *excerpts[]={
   "Alice was beginning to get very tired of sitting by her sister on the bank, and of having nothing to do: once or twice she had peeped into the book her sister was reading, but it had no pictures or conversations in it, ‘and what is the use of a book,’ thought Alice ‘without pictures or conversations?’\n\nSo she was considering in her own mind (as well as she could, for the hot day made her feel very sleepy and stupid), whether the pleasure of making a daisy-chain would be worth the trouble of getting up and picking the daisies.",
   "It is a truth universally acknowledged, that a single man in possession of a good fortune, must be in want of a wife.\n\nHowever little known the feelings or views of such a man may be on his first entering a neighbourhood, this truth is so well fixed in the minds of the surrounding families, that he is considered the rightful property of some one or other of their daughters.\n\n‘My dear Mr. Bennet,’ said his lady to him one day, ‘have you heard that Netherfield Park is let at last?’",
   "To Mrs. Saville, England.\n\nSt. Petersburgh, Dec. 11th, 17—.\n\nYou will rejoice to hear that no disaster has accompanied the commencement of an enterprise which you have regarded with such evil forebodings. I arrived here yesterday, and my first task is to assure my dear sister of my welfare and increasing confidence in the success of my undertaking.",
   "Byl pozdní večer – první máj –\nvečerní máj – byl lásky čas.\nHrdliččin zval ku lásce hlas,\nkde borový zaváněl háj.\nO lásce šeptal tichý mech;\nkvětoucí strom lhal lásky žel,\nsvou lásku slavík růži pěl,\nrůžinu jevil vonný vzdech."
  };
  if(sample<0||sample>3)return 2;
  doc.text=(char*)excerpts[sample];doc.length=strlen(doc.text);
  snprintf(doc.title,sizeof(doc.title),"%s",titles[sample]);snprintf(doc.chapters[0].title,sizeof(doc.chapters[0].title),"%s",chapters[sample]);
  doc.format=sample==3?FOLIO_FORMAT_TEXT:FOLIO_FORMAT_EPUB;
  ui.options.bionic=argc>6?atoi(argv[6]):0;
  if(argc>7)ui.options.font=atoi(argv[7]);
 }
 ui.jumpValue=1;ui.hasBookmark=true;reflow();
 if(ui.view==UI_LIBRARY||ui.view==UI_FOLDER){
  const char *names[]={"Alice's Adventures in Wonderland.epub","Pride and Prejudice.epub","Frankenstein.epub","Máj - Karel Hynek Mácha.txt"};
  ui.listCount=4;ui.selected=0;ui.libraryReady=true;snprintf(status,sizeof(status),"sdmc:/Books/");
  const int progress[]={37,12,0,-1};
  for(int i=0;i<4;i++){filtered[i]=i;snprintf(entries[i].name,sizeof(entries[i].name),"%s",names[i]);entries[i].percent=progress[i];entries[i].directory=ui.view==UI_FOLDER;}
 }
 ui_build(&ui,&controls);
 puts("SCREEN\ttop");if(ui.view==UI_LIBRARY||ui.view==UI_FOLDER)draw_library_top();else draw_reader();
 puts("SCREEN\tbottom");draw_bottom();reader_layout_free(&layout);return 0;
}
