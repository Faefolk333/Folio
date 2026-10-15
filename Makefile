.DEFAULT_GOAL := all
ifeq ($(strip $(DEVKITARM)),)
$(error Start a devkitPro shell with DEVKITARM configured)
endif
include $(DEVKITARM)/3ds_rules
APP_TITLE := Folio
APP_DESCRIPTION := Folio 0.4.5 - by Faefolk333
APP_AUTHOR := Faefolk333
APP_ICON := assets/icon48.png
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS := -std=gnu11 -O2 -g -Wall -Wextra -mword-relocations -ffunction-sections $(ARCH) -D__3DS__ -Iinclude -I$(CTRULIB)/include -I$(PORTLIBS)/include
LDFLAGS := -specs=3dsx.specs $(ARCH) -Wl,-Map,build/folio.map
LIBS := -L$(CTRULIB)/lib -L$(PORTLIBS)/lib -lcitro2d -lcitro3d -lctru -lz -lm
OBJECTS := build/main.o build/document.o build/epub.o build/startup.o build/reader.o build/ui.o build/fonts.o build/encoding.o build/i18n.o build/settings.o build/backlight.o build/settings_job.o
.PHONY: all cia
MAKEROM ?= makerom
BANNERTOOL ?= bannertool
all: folio.3dsx
build:
	mkdir -p build
build/%.o: source/%.c include/folio.h Makefile | build
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
build/fonts.o: source/fonts.S $(wildcard assets/fonts/*.bcfnt) | build
	$(CC) $(ARCH) -c $< -o $@
folio.smdh: Makefile
folio.elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@
folio.3dsx: folio.elf folio.smdh
	3dsxtool folio.elf $@ --smdh=folio.smdh
build/folio-banner.bin: assets/cia/banner.png assets/cia/silence.wav | build
	$(BANNERTOOL) makebanner -i assets/cia/banner.png -a assets/cia/silence.wav -o $@
folio.cia: folio.elf folio.smdh build/folio-banner.bin assets/cia/folio.rsf Makefile
	$(MAKEROM) -f cia -o $@ -target t -rsf assets/cia/folio.rsf -elf folio.elf -icon folio.smdh -banner build/folio-banner.bin -ver 5
cia: folio.cia
-include $(OBJECTS:.o=.d)
