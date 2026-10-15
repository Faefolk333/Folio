.DEFAULT_GOAL := all
ifeq ($(strip $(DEVKITARM)),)
$(error Start a devkitPro shell with DEVKITARM configured)
endif
include $(DEVKITARM)/3ds_rules
APP_TITLE := Folio
APP_DESCRIPTION := Folio 0.4.3 - recent books and reading controls
APP_AUTHOR := Folio contributors
APP_ICON := assets/icon48.png
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS := -std=gnu11 -O2 -g -Wall -Wextra -mword-relocations -ffunction-sections $(ARCH) -D__3DS__ -Iinclude -I$(CTRULIB)/include -I$(PORTLIBS)/include
LDFLAGS := -specs=3dsx.specs $(ARCH) -Wl,-Map,build/folio.map
LIBS := -L$(CTRULIB)/lib -L$(PORTLIBS)/lib -lcitro2d -lcitro3d -lctru -lz -lm
OBJECTS := build/main.o build/document.o build/epub.o build/startup.o build/reader.o build/ui.o build/fonts.o build/encoding.o build/i18n.o build/settings.o build/backlight.o build/settings_job.o
.PHONY: all
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
-include $(OBJECTS:.o=.d)
