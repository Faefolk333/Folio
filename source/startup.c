#include "startup.h"
#include <3ds.h>
#include <stdio.h>

static PrintConsole bootConsole;

void startup_log(const char *message) {
    /* SD writes must never be a dependency of startup or diagnostics. */
    (void)message;
}

void startup_begin(void) {
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, &bootConsole);
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("FOLIO 0.4.6\nby Faefolk333\nHold Y: skip automatic SD scan\n\n");
    printf("Starting...\n\n");
    gfxFlushBuffers();
    gfxSwapBuffers();
}

bool startup_stage(const char *message) {
    printf("%s\n", message);
    fflush(stdout);
    gfxFlushBuffers();
    /* Display only: never hide HID, SD I/O or an APT wait behind a marker. */
    return true;
}

void startup_error(const char *message) {
    startup_log(message);
    printf("\nERROR: %s\n\nPress START to exit.\n", message);
    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & KEY_START) break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
}

void startup_use_gpu(void) {
    /* consoleInit uses RGB565/single buffering; citro2d transfers RGB8. */
    gfxSetScreenFormat(GFX_BOTTOM, GSP_BGR8_OES);
    gfxSetDoubleBuffering(GFX_BOTTOM, true);
    /* Do not print through bootConsole after the framebuffer format changes. */
}
