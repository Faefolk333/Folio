Folio 0.4.8 by **Faefolk333** — HOME close fix candidate.

The owner confirmed that 0.4.7 exits immediately with START, but HOME Menu's
Close Software still hangs. This build addresses that separate suspended-close
path; **hardware confirmation is still required**.

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
checks passed. Mocks cannot prove the real console's APT behavior. This release
is a **prerelease** until HOME closing is verified on hardware.
Until then, **START is the owner-confirmed way to exit 0.4.7**.

### Install and test

Extract **Folio-0.4.8-SD.zip** to the SD root, preserving your own `/Folio/`
preferences and progress. Install **folio-0.4.8-test.cia** over the existing title
with FBI. Do not delete the application or saved data first.
For Homebrew Launcher use the included `folio.3dsx`.

Test HOME → return to Folio, then HOME → Close Software, and START. Repeat with
a book open and the lower display asleep. Reopen the book to check the position.
Report which step fails rather than waiting several minutes.

TXT, HTML and text-focused DRM-free EPUB; English/Czech interface, five themes,
six fonts, Bionic Reading, Focus Line, recent-first library and per-book saves.
No PDF, DRM, complex CSS or embedded covers. Typography is session-only.
A stalled final SD write may lose the last checkpoint; checkpoint waiting is
limited to 500 ms. Other in-app SD operations can still be slow.

Substantially developed with AI assistance (OpenAI Codex). Preview images are
labelled host-rendered previews, not hardware screenshots. No AI bitmap artwork
or complete books are bundled.
