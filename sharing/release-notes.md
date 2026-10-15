Folio 0.4.8 by **Faefolk333** — HOME close fix.

The owner confirmed successful launch and immediate HOME-menu closing in 0.4.8
on a real New Nintendo 3DS, resolving the reported 0.4.7 Close Software hang.

### Changes

- Bounded reading checkpoint before HOME suspension.
- CIA-only guard handles a sustained system close request while the main
  thread is suspended inside APT. Uses the normal APT application-close commands,
  with atomic ownership so main and guard cannot close concurrently.
- If APT returns a cancelled wakeup without restoring graphics, close directly
  instead of entering GPU teardown.
- HOME alone does not exit. START and Homebrew Launcher keep normal cleanup.
- Same book icon, Homebrew launch logo, Title ID, permissions and save format.
  CIA install revision **8** (0.0.8).

Native builds, nine mocked HOME-state cases, bounded-save tests and package/logo
checks passed. Launch and immediate HOME-menu closing were subsequently
confirmed by the owner on the console. Other hardware scenarios are not
implicitly covered by that confirmation.

### Install or update

Extract **Folio-0.4.8-SD.zip** to the SD root, preserving your own `/Folio/`
preferences and progress. Install **folio.cia** over the existing title
with FBI. Do not delete the application or saved data first.
For Homebrew Launcher use the included `folio.3dsx`.

This is the same executable as the confirmed 0.4.8 test build; only the download
name and release documentation changed. No reinstall is needed if you already
have that build.

TXT, HTML and text-focused DRM-free EPUB; English/Czech interface, five themes,
six fonts, Bionic Reading, Focus Line, recent-first library and per-book saves.
No PDF, DRM, complex CSS or embedded covers. Typography is session-only.
A stalled final SD write may lose the last checkpoint; checkpoint waiting is
limited to 500 ms. Other in-app SD operations can still be slow.

Substantially developed with AI assistance (OpenAI Codex). Preview images are
labelled host-rendered previews, not hardware screenshots. No AI bitmap artwork
or complete books are bundled.
