#include "i18n.h"
#include <string.h>
static int currentLanguage;
void i18n_set(int language){currentLanguage=language==LANG_CS?LANG_CS:LANG_EN;}
const char *tr(const char *english,const char *czech){return currentLanguage==LANG_CS?czech:english;}
const char *i18n_error(const char *english){
 static const struct {const char *en,*cs;} errors[]={
 {"Invalid or unsupported EPUB archive.","Neplatný nebo nepodporovaný EPUB."},
 {"Encrypted/DRM EPUB is not supported.","Šifrovaný EPUB / DRM není podporován."},
 {"Unsupported EPUB chapter format.","Nepodporovaný formát kapitoly EPUB."},
 {"Cannot decode chapter: encoding, corrupt data or 4 MiB limit.","Chyba kapitoly: kódování, poškození nebo limit 4 MiB."},
 {"EPUB text exceeds 4 MiB.","Text EPUB překračuje 4 MiB."},
 {"EPUB contains no readable text.","EPUB neobsahuje čitelný text."},
 {"Out of memory.","Nedostatek paměti."},
 {"PDF support is planned.","Podpora PDF je plánována."},
 {"Use TXT, HTML, EPUB or PDF.","Použij TXT, HTML nebo EPUB."},
 {"Cannot read file (or it is larger than 4 MiB).","Soubor nelze číst nebo překračuje 4 MiB."},
 {"Unsupported or invalid text encoding.","Nepodporované nebo chybné kódování textu."},
 {"Choose your books folder.","Vyber složku se svými knihami."},
 {"Cannot open SD folder.","Složku na SD nelze otevřít."},
 {"First 256 entries only; split books into folders.","Pouze prvních 256 položek; rozděl knihy do složek."},
 {"Book layout exceeds available memory.","Kniha je příliš velká pro dostupnou paměť."},
 {"Settings could not be saved. Check SD:/Folio.","Nastavení nelze uložit. Zkontroluj SD:/Folio."},
 {"Settings could not be saved. Folder works for this session.","Nastavení neuloženo. Složka platí pro tento běh."},
 {"Settings I/O pending; please wait.","Probíhá ukládání nastavení; vyčkej."},
 {"Settings I/O is slow. START exits without waiting.","Ukládání se zdrželo. START ukončí bez čekání."},
 {"Folder saved. Library refreshed.","Složka uložena. Knihovna obnovena."},
 {"Safe start: automatic scan skipped.","Bezpečný start: automatické načtení přeskočeno."},
 {"Backlight service unavailable; screen is only black.","Podsvícení nelze vypnout; displej je pouze černý."}
 };
 for(unsigned i=0;i<sizeof(errors)/sizeof(errors[0]);i++)if(!strcmp(english,errors[i].en))return tr(errors[i].en,errors[i].cs);
 return english; /* Book titles, folder paths and filenames are never translated. */
}
