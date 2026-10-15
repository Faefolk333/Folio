Folio is a customizable e-reader for New Nintendo 3DS, launched through Homebrew
Launcher. Read TXT, HTML and DRM-free EPUB files from your SD card.

### In 0.4.3

- Recently opened/resumed books appear first; order persists with reading state.
- Left/right selects books. All/EPUB/TXT/HTML filters use the touchscreen.
- Circle Pad up/down scrolls text, including with Focus Line enabled.
- D-pad up/down scrolls on the main reader and navigates controls in submenus.
- L/ZL turns to the previous page, R/ZR to the next; brightness uses touch +/-.

### Install or update

Extract **Folio-0.4.3-SD.zip** to your SD root, preserving existing reading-state
and preference files. The full ZIP supplies `/Folio/progress/`; it is required
to save positions. Launch `/3ds/folio/folio.3dsx` from Homebrew Launcher.
Choose your books folder in Library. Exit with START after settings finish saving.

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
