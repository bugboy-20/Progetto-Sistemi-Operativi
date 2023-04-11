PREFIX = mipsel-linux-gnu-
CC = $(PREFIX)gcc
LD = $(PREFIX)ld

UDIR = /usr/include
SDIR = /usr/share
PANDOS_HEADERS = ../include
SOURCE = src
BUILD = build
CFLAGS = -ffreestanding -ansi -mips1 -mabi=32 -std=gnu99 -mno-gpopt -EL -G 0 -mno-abicalls -fno-pic -mfp32 -I$(UDIR) -I$(PANDOS_HEADERS) -Wall -O0
LFLAGS = -G 0 -nostdlib -T $(SDIR)/umpscore.ldscript -m elf32ltsmip
OBJECTS = $(BUILD)/p1test.o $(BUILD)/pcb.o $(BUILD)/ash.o $(BUILD)/ns.o $(BUILD)/libumps.o $(BUILD)/crtso.o

all: kernel.core.umps

kernel.core.umps: kernel
	umps3-elf2umps -k kernel

kernel: $(OBJECTS)
	$(LD) -o kernel $(OBJECTS) $(LFLAGS)

$(BUILD)/p1test.o: p1test.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/p1test.o p1test.c

$(BUILD)/crtso.o: $(SDIR)/crtso.S
	$(CC) $(CFLAGS) -c -o $(BUILD)/crtso.o $(SDIR)/crtso.S

$(BUILD)/libumps.o: $(SDIR)/libumps.S
	$(CC) $(CFLAGS) -c -o $(BUILD)/libumps.o $(SDIR)/libumps.S

$(BUILD)/pcb.o: $(SOURCE)/pcb.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/pcb.o $(SOURCE)/pcb.c

$(BUILD)/ash.o: $(SOURCE)/ash.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/ash.o $(SOURCE)/ash.c

$(BUILD)/ns.o: $(SOURCE)/ns.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/ns.o $(SOURCE)/ns.c

clean:
	rm kernel *.umps -r $(BUILD)/*
