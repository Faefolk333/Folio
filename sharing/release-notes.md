Folio by **Faefolk333** is a customizable TXT, HTML and DRM-free EPUB e-reader
for New Nintendo 3DS.

### New in 0.4.7

- Safer Close Software handling: no new loading frame after APT requests exit,
  and busy frames reject close requests.
- Final reading-position checkpoint uses a private FS worker with at most
  500 ms of waiting. A stalled SD write no longer blocks this exit step forever.
- Keeps the Homebrew launch logo from 0.4.6, which the owner confirmed launches
  successfully on a real New 3DS.
- HOME banner uses only the supplied book icon and Folio name. Faefolk333 stays
  credited at startup, in-app and in publisher metadata.
- Same Title ID and compatible reading data; CIA installation revision 7.

**Verification:** native builds, package/hash checks and host shutdown tests
passed, including a simulated stalled SD write. The new HOME closing behavior
has not yet been verified on hardware; no guaranteed device fix is claimed.
The 500 ms limit applies to final checkpoint I/O, not the entire system shutdown.
If the SD write times out, the last unsaved reading position may be lost. Return
to Library to save before closing if needed. Other synchronous SD operations
during normal use can still be slow.

### Install or update

Extract **Folio-0.4.7-SD.zip** to the SD root, preserving existing preference
files and everything in your own `/Folio/progress/`. Install **folio.cia** over
the existing title with FBI on a CFW-enabled New Nintendo 3DS, or launch
`/3ds/folio/folio.3dsx` through Homebrew Launcher. The bare CIA does not create
the data directory or copy font notices; extract the full SD ZIP first.

Title ID: `000400000FF04100`. CIA install revision: **7** (0.0.7).
HOME opens HOME Menu in CIA; choose Close Software to exit. START also exits.
The 3DSX version uses START to return to Homebrew Launcher.

Older release downloads have been removed at the owner's request to avoid
installing obsolete builds. Source history remains available in the repository.

### Features and limits

Five themes, six font choices, adjustable typography, Bionic Reading, Focus Line,
recent-first library, per-book positions/bookmarks and chapter/page jumps.
English by default, selectable Czech. L/R and ZL/ZR turn pages; Circle Pad
scrolls lines. Touch or SELECT wakes the lower display.

Text-focused EPUB only: no DRM, PDF, embedded covers or complex CSS layouts.
Nonrecursive library, up to 256 entries. Startup opens Library and restores
saved position when opening a book. Font/style settings are session-only.

Substantially developed with AI assistance (OpenAI Codex). Sharing images are
explicitly labelled host-rendered previews, not console screenshots. No
AI-generated bitmap artwork or complete books are bundled.
