# Folio sharing materials / 0.4.4

Application author: **Faefolk333**.

description-en.md contains short and long English copy; description-cs.md is
the Czech counterpart. CIA packaging is available; neither claims PDF, real cover artwork,
persistent typography or automatic opening of the last book.

## Image provenance

previews/ contains eight 800x1040 PNG images and overview.png. Each image is
explicitly labelled "Host-rendered preview". These execute Folio's native
draw_reader/draw_library_top/draw_bottom and UI code on the host, using real
BCFNT advance metrics but approximate TTF rasterization. They are NOT hardware
or emulator screenshots. A scene shows demonstration progress values and short
book excerpts; it does not assert that full editions are bundled with Folio.
Do not relabel these as actual console screenshots in a release listing.

01 library; 02 reading / Paper; 03 themes; 04 typography; 05 settings;
06 Bionic + Focus / Dark; 07 chapter jump / Red Night; 08 Czech / Máj.

The sample texts are opening excerpts of public-domain originals:

- Lewis Carroll, Alice's Adventures in Wonderland (1865):
  https://www.gutenberg.org/ebooks/11
- Jane Austen, Pride and Prejudice (1813):
  https://www.gutenberg.org/cache/epub/1342/pg1342.html
- Mary Shelley, Frankenstein (1818):
  https://www.gutenberg.org/files/84/84-h/84-h
- Karel Hynek Mácha, Máj (1836), first eight lines:
  https://cs.wikisource.org/wiki/M%C3%A1j_(M%C3%A1cha)/1

No modern book covers, translations, publisher artwork or AI images are used.
The actual supplied application icon is included as icon48.png.

## Reproduce

Build build/preview.exe using the command in BUILD-NOTES.md, then run:
python tests/share_previews.py
Python requires Pillow. Rendering does not run the 3DS GPU or verify hardware.

## Before publishing to Universal-DB / Universal-Updater

Provide a public project/release URL and an author credit, choose the project's
code license, and confirm redistribution rights for the user-supplied icon.
This workspace currently has no top-level code LICENSE file. Existing font
license notices must accompany redistribution. No arbitrary license or author
identity has been selected on the user's behalf.

These materials accompany the GitHub release. Publication on GitHub does not
mean the application has been submitted to or accepted by Universal-DB.
For authentic screenshots, capture the same scenes from a working console or
emulator and replace the host previews. This session had neither connected
3DS capture nor a configured 3DS emulator available.
