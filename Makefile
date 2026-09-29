# GBA build - requires devkitARM (https://devkitpro.org/wiki/Getting_Started)
TARGET := danny_steel_title
TITLE  := DANNYSTEEL

DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
export DEVKITPRO DEVKITARM
export PATH := $(DEVKITARM)/bin:$(DEVKITPRO)/tools/bin:$(PATH)

# Check the compiler by its full path. (`which` inside $(shell) does NOT see the PATH exported above on
# older GNU make, which made this check fail even inside the devkitARM container.)
ifeq ($(wildcard $(DEVKITARM)/bin/arm-none-eabi-gcc*),)
$(error arm-none-eabi-gcc not found in $(DEVKITARM)/bin. Install devkitARM or set DEVKITPRO=/path/to/devkitpro)
endif

CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
ARCH    := -mthumb -mthumb-interwork -mcpu=arm7tdmi
CFLAGS  := $(ARCH) -O2 -Wall -std=gnu11 -fomit-frame-pointer -Isrc
LDFLAGS := $(ARCH) -specs=gba.specs
SRC     := $(wildcard src/*.c)
HDR     := $(wildcard src/*.h)

all: $(TARGET).gba

$(TARGET).elf: $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $@

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@if which gbafix >/dev/null 2>&1; then gbafix $@ -t$(TITLE); \
	else echo "warning: gbafix not found; ROM header not fixed"; fi

clean:
	rm -f $(TARGET).elf $(TARGET).gba

.PHONY: all clean
