PREFIX = mipsel-linux-gnu-
CC = $(PREFIX)gcc
LD = $(PREFIX)ld

UDIR = /usr/include/umps3
SDIR = /usr/share/umps3
PANDOS_HEADERS = ./include
SOURCE = src
BUILD = build
TEST_FILE = p2test.01
CFLAGS = -ffreestanding -ansi -mips1 -mabi=32 -std=gnu99 -mno-gpopt -EL -G 0 -mno-abicalls -fno-pic -mfp32 -I$(UDIR) -I$(PANDOS_HEADERS) -Wall -O0
LFLAGS = -G 0 -nostdlib -T $(SDIR)/umpscore.ldscript -m elf32ltsmip
OBJECTS = $(BUILD)/$(TEST_FILE).o $(BUILD)/exceptions.o $(BUILD)/initial.o $(BUILD)/interrupts.o $(BUILD)/scheduler.o $(BUILD)/syscall.o $(BUILD)/pcb.o $(BUILD)/ash.o $(BUILD)/ns.o $(BUILD)/libumps.o $(BUILD)/crtso.o

all: kernel.core.umps

kernel.core.umps: kernel
	umps3-elf2umps -k kernel

kernel: $(OBJECTS)
	$(LD) -o kernel $(OBJECTS) $(LFLAGS)

$(BUILD)/$(TEST_FILE).o: $(TEST_FILE).c
	$(CC) $(CFLAGS) -c -o $(BUILD)/$(TEST_FILE).o $(TEST_FILE).c

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

$(BUILD)/exceptions.o: $(SOURCE)/exceptions.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/exceptions.o $(SOURCE)/exceptions.c

$(BUILD)/initial.o: $(SOURCE)/initial.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/initial.o $(SOURCE)/initial.c

$(BUILD)/interrupts.o: $(SOURCE)/interrupts.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/interrupts.o $(SOURCE)/interrupts.c

$(BUILD)/scheduler.o: $(SOURCE)/scheduler.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/scheduler.o $(SOURCE)/scheduler.c

$(BUILD)/syscall.o: $(SOURCE)/syscall.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/syscall.o $(SOURCE)/syscall.c

clean:
	rm kernel *.umps -r $(BUILD)/*
