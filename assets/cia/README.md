# Folio CIA packaging

Author/publisher: **Faefolk333**. The SMDH publisher field is shared by the
3DSX and CIA builds. The HOME banner uses the existing owner-supplied icon,
Folio's name and Faefolk333's name with a short silent PCM track.
No AI bitmap generator or proprietary banner assets are used.

## Build

First build `make`, then `make cia`. Put makerom 0.19.0 and bannertool 1.2.2 on
PATH, or override `MAKEROM=/path/to/makerom` and `BANNERTOOL=/path/to/bannertool`.
The same devkitARM/libctru ELF feeds both formats. Release tools:

- https://github.com/3DSGuy/Project_CTR/releases/tag/makerom-v0.19.0
- https://github.com/Epicpkmn11/bannertool/releases/tag/v1.2.2
- https://github.com/3DSGuy/Project_CTR/releases/tag/ctrtool-v1.3.0

Rebuild the deterministic banner with `python tests/build_cia_assets.py` (Pillow).
The committed banner and WAV allow building without Python.

## Identity and access

Stable application Title ID: `000400000FF04100` (UniqueId `0xFF041`).
Product code: `CTR-P-FOLI`. This is a locally selected homebrew identifier,
not an official Nintendo allocation; keep it stable for updates.
App version 0.4.4 uses CIA installation revision 4 (shown by ctrtool as 0.0.4).
The CIA installation revision is a separate monotonically increasing number.
The New 3DS configuration requests 124MB, 804MHz and L2 cache.
The RSF requests SD access and services needed for graphics, input, font/config,
APT and the optional LCD backlight. It does not request NAND filesystem access,
extdata or SVC backdoors. All application-owned files remain below `/Folio/`.

CIA installation requires a CFW-enabled console and a CIA installer. Extract
the release SD ZIP to supply `/Folio/progress/` and font notices before launch.
Installed HOME handling differs from 3DSX launcher handling. A build/structural
check cannot substitute for an installation/launch test on a console.
