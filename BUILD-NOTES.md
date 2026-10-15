# Folio 0.4.5 — verification

Author: Faefolk333. Current CIA packaging is documented in assets/cia/README.md.
The 3DSX and CIA share the same SMDH author field and native libctru ELF.
Run `make` then `make cia` with makerom 0.19.0 and bannertool 1.2.2.
`python tests/package_test.py` checks all populated-language SMDH authors,
embedded 3DSX SMDH, CIA content/NCCH/ExeFS hashes, title identity, installation
revision 5, valid launch priority, matching launch/program IDs and exact CIA icon
equivalence to the standalone SMDH.
Nintendo retail signature checks are expected to fail for the homebrew test-key
CIA; CFW is required. Hash checks are independent of signature trust.
CIA installation, launch and HOME transitions have not been hardware-tested.

0.4.4 has a user-reported launch error, "SD card was removed". Independent
decoding found its main-thread priority was 80. makerom adds 32 to the RSF
Application priority; setting Priority: 48 was invalid. Priority: 16 encodes
48, matching the standard application example. The 0.4.5 regression check
rejects the archived 0.4.4 CIA at priority 80 and accepts the corrected CIA.
Jump ID already matched the installed Title ID, so it was not changed.
The causal connection to this specific error still needs a console retest.
makerom behavior: https://github.com/3DSGuy/Project_CTR/blob/master/makerom/src/exheader.c
Reference app: https://github.com/Universal-Team/Universal-Updater/blob/master/app/build-cia.rsf

Folder-save hotfix: see FOLDER-FIX-0.4.1.txt. Preferences saves now use a private
native FS session on a worker, queued after rendering the chosen library.
No UI-thread preferences writes; synchronous initial settings reads are unchanged.
Host native-backend fault-injection test:

```sh
/usr/bin/gcc -std=gnu11 -Wall -Wextra -Werror -pthread -D__3DS__ -DFOLIO_JOB_TEST -Iinclude tests/settings_job_test.c source/settings_job.c source/settings.c -o build/settings-job-test.exe
./build/settings-job-test.exe
```

Built with devkitARM r68 / GCC 16.1, libctru 2.7, citro2d/citro3d and 3ds-zlib.
Run make in the devkitPro shell. No source C warnings; the linker emits its
supplied crtn.o GNU-stack warning.

This revision has not been tested on physical 3DS hardware here. Earlier startup
fixes were confirmed by the user. SD settings/refresh happen after the first
rendered frame, not in the boot-console sequence. Hold Y to skip that automatic
access. No startup mkdir or writes.

## Release 0.4.3

Book state appends a fifth unsigned 64-bit field: reading-order timestamp from
osGetTime. Ties/backward clock changes within a scanned library advance the
latest observed order by one. Old 3/4-field saves read with order zero; unread
and equal-order entries use a deterministic alphabetical/path tie-breaker.
Open/resume sets dirty and saves even when no page has been turned.
No new archive paths, folders or settings-record version changes.

The native control mapping uses L/ZL and R/ZR for page turns. Circle Pad up/down
always scrolls text. D-pad up/down scrolls on UI_HOME and navigates controls in
submenus. Single-line scrolling preserves the highlighted screen row. Reading
directions do not wake the lower display or postpone its sleep timer on UI_HOME.
Library left/right selects books; format changes are touchscreen-only.

Additional checks (MSYS shell, from root):

```sh
/usr/bin/gcc -std=gnu11 -Iinclude tests/library_order.c source/reader.c source/ui.c source/i18n.c source/document.c source/epub.c source/encoding.c -lz -lm -o build/library-order-test.exe
./build/library-order-test.exe
python tests/share_previews.py
```

sharing/previews images use native rendering functions with approximate host
rasterization. They are labelled as previews and are not console screenshots.
No real-console verification of the new controls or persistent order performed.

## Release 0.4.2

Native ARM build and host reader/UI, EPUB, encoding, backlight and asynchronous
settings regressions passed. Added visible-bookmark boundary tests and both
bookmark button labels. Reviewed host-rendered Type/themes/bookmark UI.
No real-console timing, HOME, or freeze verification was available.

Widths of codepoints below 2048 are cached for the selected font/size.
Unicode composition lookup runs only for U+0300..U+036F combining marks;
the generated Latin composition table uses marks within this range.
Returning to Library updates only the current book's cached percentage;
Refresh still reloads all on-disk percentages. Existing book reopening reuses
the loaded document. No persistent book-text cache or new file format.

Loading feedback is a rendered frame before each synchronous operation, NOT
a background parser or cancellation mechanism. Native SD stalls can still
block input. This is deliberately documented in the release notes.

The local libctru apt.h API aptSetHomeAllowed(false) rejects normal HOME
transitions; aptCheckHomePressRejected() displays a six-second START instruction.
START is still the exit route. Forced system exit/sleep retains APT handling.

## Backlight

Acquisition is lazy, on first sleep, not on launch. Check for a loader-provided
or registered gsp::Lcd service; use srvSetBlockingPolicy(true) during gspLcdInit.
Only GSPLCD_SCREEN_BOTTOM is passed to PowerOff/PowerOn. Restore on wake,
suspend and exit. No permission bypass or system filesystem writes.

This avoids waiting for an unregistered/full service port; it does not impose
timeouts on every IPC or filesystem operation. Refusal is reported on the top
screen and software blanking remains as a fallback. Mock tests validate the
calls/lifecycle, not hardware permissions or actual illumination.

API references:
- https://github.com/devkitPro/libctru/blob/master/libctru/source/services/gsplcd.c
- https://github.com/devkitPro/libctru/blob/master/libctru/source/srv.c

## Library and language

Top-screen cards use filenames and rendered shapes, not real EPUB artwork.
The selected folder is nonrecursive, with a maximum 256 directory entries.
Saved progress is read without parsing all books. The fourth state field holds
text length and the fifth holds reading order. Old three/four-field saves remain
readable; percentages/order become available after the next successful save.

Language/folder settings alternate two checksummed records inside /Folio.
Default English, selectable Czech. Ordinary UI/error messages are translated;
startup diagnostics remain English. Book content is never translated. Other
reading preferences are session-only.

## Host checks

Run in MSYS from project root (Python/Pillow for optional preview):

```sh
/usr/bin/gcc -std=c11 -Wall -Wextra -Werror -Iinclude tests/reader_ui.c source/reader.c source/ui.c source/i18n.c -lm -o build/reader-ui-test.exe
./build/reader-ui-test.exe
/usr/bin/gcc -std=c11 -Wall -Wextra -Werror -DFOLIO_LCD_TEST -Iinclude tests/backlight_test.c source/backlight.c -o build/backlight-test.exe
./build/backlight-test.exe
/usr/bin/gcc -std=c11 -Wall -Wextra -Werror -Iinclude tests/settings_test.c source/settings.c -o build/settings-test.exe
python -c "import tempfile,subprocess; d=tempfile.TemporaryDirectory(); subprocess.run(['build/settings-test.exe',d.name],check=True); d.cleanup()"
/usr/bin/gcc -std=gnu11 -Iinclude tests/parser.c source/document.c source/epub.c source/encoding.c -lz -o build/parser-test.exe
python tests/epub_test.py
python tests/encoding_test.py
/usr/bin/gcc -Iinclude tests/font_test.c source/reader.c -lm -o build/font-test.exe
./build/font-test.exe
/usr/bin/gcc -std=gnu11 -Iinclude tests/preview.c source/reader.c source/ui.c source/i18n.c source/document.c source/epub.c source/encoding.c -lz -lm -o build/preview.exe
python tests/preview.py
```

Coverage: UI bounds/hit areas in both languages, English label coverage,
library selection and page bounds, filters, return view, language switching,
UTF-8 anchors/reflow/focus, Bionic, torn settings-write recovery, LCD
sleep/wake/resume/exit and errors, EPUB/coding regressions, Czech font glyphs.

Visual checks capture native drawing functions and real BCFNT advances with
approximate host rasterization. They are not 3DS GPU screenshots.

On-device checks still required: saved-folder restart, percentages,
touch/SELECT wake, reading controls while dark, lid close/open and exiting
to Homebrew Launcher with the lower light restored.
