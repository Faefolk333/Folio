Folio by **Faefolk333** is a customizable e-reader for New Nintendo 3DS.
Read TXT, HTML and DRM-free EPUB files from your SD card.

### New in 0.4.4

- Native installable CIA plus the Homebrew Launcher 3DSX build.
- Faefolk333 author credit in both formats, the boot screen, Type screen,
  HOME banner, guides and release materials.
- Installed CIA enables normal HOME handling; 3DSX keeps START-to-exit behavior.
- No loading frames are submitted after APT releases graphics access.

CIA packaging, hashes and metadata have been checked. Installation, launch,
HOME suspend/resume and persistence still require a real-console test.

### In 0.4.3

- Recently opened/resumed books appear first; order persists with reading state.
- Left/right selects books. All/EPUB/TXT/HTML filters use the touchscreen.
- Circle Pad up/down scrolls text, including with Focus Line enabled.
- D-pad up/down scrolls on the main reader and navigates controls in submenus.
- L/ZL turns to the previous page, R/ZR to the next; brightness uses touch +/-.

### Install or update

Extract **Folio-0.4.4-SD.zip** to your SD root, preserving existing reading-state
and preference files. The full ZIP supplies `/Folio/progress/`; it is required
to save positions. Launch `/3ds/folio/folio.3dsx` from Homebrew Launcher.
Choose your books folder in Library. Exit with START after settings finish saving.
For CIA, install `/cias/folio.cia` from the full SD ZIP on a CFW-enabled New
Nintendo 3DS. A bare CIA does not create `/Folio/progress/` or copy font notices.
The selected homebrew Title ID is `000400000FF04100`. App version: 0.4.4;
CIA install revision: 4 (inspection tools display this as 0.0.4).

### Notes

English is the default; Czech can be selected in Settings. Five themes, six font
choices, Bionic Reading, Focus Line, per-book positions/bookmarks and chapter/page
jumps are available. Startup opens Library; saved positions restore when opening
a book. Typography is session-only. EPUB is text-focused; DRM, PDF, cover artwork
and complex CSS are not supported. Library scanning is nonrecursive, up to 256
entries. Synchronous SD calls may still stall input.

Native ARM build and host checks passed. Latest controls/order need verification
on a real console. The project was substantially developed with AI assistance
(OpenAI Codex). Images in the sharing archive are labelled host-rendered previews,
not device screenshots. No generated bitmap artwork or full books are bundled.
