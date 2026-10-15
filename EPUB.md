# EPUB support

The reader follows META-INF/container.xml to the OPF package, resolves manifest
references relative to that package and reads linear spine items in order.
EPUB 2 and EPUB 3 reflowable UTF-8 XHTML are supported as text. The document model
stores each chapter's offset in the combined text, so the existing progress
storage and chapter menu also work for EPUB.

ZIP stored and deflate methods are supported through zlib. Entry lengths,
archive bounds and CRC checks are enforced. Files are read directly from the
archive; nothing is extracted to the SD card. Script, style and head content
are excluded from chapter text.

Limits and unsupported features are listed in BUILD-NOTES.md. This is a
text-focused subset, not a fully conforming EPUB reading system. Reading order
reference: https://www.w3.org/TR/epub-33/

Install the additional devkitPro dependency with:

    pacman -S 3ds-zlib

Host parser verification (MSYS gcc and zlib-devel):

    gcc -std=gnu11 -Wall -Wextra -Iinclude tests/parser.c source/document.c source/epub.c -lz -o build/parser-test.exe
    python tests/epub_test.py
