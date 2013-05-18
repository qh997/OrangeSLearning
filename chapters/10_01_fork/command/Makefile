ASM := nasm
CC := gcc
LD := ld

SHOW_ASMCPL := @echo -n ' COMPILE   ASM    '; echo
SHOW_CCCPL  := @echo -n ' COMPILE    CC    '; echo 
SHOW_LDLNK  := @echo -n ' LINK       LD    '; echo
SHOW_INSTAL := @echo -n ' INSTALL  '; echo

CMD_PATH := command
LIB_PATH := lib

OUT_DIR     := ../out
OUT_CMD_DIR := $(OUT_DIR)/$(CMD_PATH)
OUT_LIB_DIR := $(OUT_DIR)/$(LIB_PATH)

MKDIR_OUT     := @mkdir -p $(OUT_DIR)
MKDIR_OUT_CMD := @mkdir -p $(OUT_CMD_DIR)

ASMFLAGS := -I ../include/ -I ../include/sys -f elf32
GCCFLAGs := -I ../include/ -I ../include/sys -c -m32 -std=c99 -fno-builtin -nostdlib -Wall
LDFLAGS  := -Ttext 0x1000 -m elf_i386

RTLIB := $(OUT_LIB_DIR)/qhcrt.a

ECHO_BIN := $(OUT_CMD_DIR)/echo

.PHONY: everytning

everytning: init $(ECHO_BIN)

all: clean everytning install

init:
	@echo build commands
	@echo
	$(MKDIR_OUT)
	$(MKDIR_OUT_CMD)

clean:
	rm $(OUT_CMD_DIR)/start.o
	rm $(OUT_CMD_DIR)/echo.o
	rm $(ECHO_BIN)

install:

$(OUT_CMD_DIR)/start.o: start.asm
	$(SHOW_ASMCPL) $@
	@$(ASM) $(ASMFLAGS) -o $@ $<

$(OUT_CMD_DIR)/echo.o: echo.c
	$(SHOW_CCCPL) $@
	@$(CC) $(GCCFLAGs) -o $@ $<

$(ECHO_BIN): $(OUT_CMD_DIR)/start.o $(OUT_CMD_DIR)/echo.o $(RTLIB)
	$(SHOW_LDLNK) $@
	@$(LD) $(LDFLAGS) -o $@ $^
