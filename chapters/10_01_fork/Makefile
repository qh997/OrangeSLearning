ASM := nasm
CC  := gcc
LD  := ld
AR  := ar

SHOW_ASMCPL := @echo -n ' COMPILE   ASM    '; echo
SHOW_CCCPL  := @echo -n ' COMPILE    CC    '; echo 
SHOW_LDLNK  := @echo -n ' LINK       LD    '; echo
SHOW_ARPL   := @echo -n ' COMPILE    AR    '; echo 
SHOW_INSTAL := @echo -n ' INSTALL  '; echo

ENTRYPOINT := 0x30400

KERNEL_PATH := kernel
LIB_PATH    := lib

OUT_DIR        := out
OUT_KERNEL_DIR := $(OUT_DIR)/$(KERNEL_PATH)
OUT_FS_DIR     := $(OUT_DIR)/fs
OUT_LIB_DIR    := $(OUT_DIR)/$(LIB_PATH)
OUT_MM_DIR     := $(OUT_DIR)/mm

MKDIR_OUT        := @mkdir -p $(OUT_DIR)
MKDIR_OUT_KERNEL := @mkdir -p $(OUT_KERNEL_DIR)
MKDIR_OUT_FS     := @mkdir -p $(OUT_FS_DIR)
MKDIR_OUT_LIB    := @mkdir -p $(OUT_LIB_DIR)
MKDIR_OUT_MM     := @mkdir -p $(OUT_MM_DIR)

IMG_FILE := boot.img
IMG_MOUNT := /mnt/floppy

HEADS := include/sys/config.h \
         include/sys/console.h \
         include/sys/const.h \
         include/sys/fs.h \
         include/sys/global.h \
         include/sys/hd.h \
         include/sys/keyboard.h \
         include/sys/keymap.h \
         include/sys/proc.h \
         include/sys/protect.h \
         include/sys/proto.h \
         include/sys/tty.h \
         include/string.h \
         include/type.h \
         include/stdio.h

BOOT_PATH := boot
BOOT_INC := $(BOOT_PATH)/include
BOOT_SRC := $(BOOT_PATH)/boot.asm
BOOT_HDR := $(BOOT_INC)/fat12.inc \
            $(BOOT_INC)/lib_rm.inc
BOOT_BIN := $(OUT_DIR)/boot.bin

LOADER_SRC := $(BOOT_PATH)/loader.asm
LOADER_HDR := $(BOOT_INC)/fat12.inc \
              $(BOOT_INC)/lib_rm.inc \
              $(BOOT_INC)/ptmode.inc
LOADER_BIN := $(OUT_DIR)/loader.bin

KERNEL_INC := include
KERNEL_ASM_FLAG := -I $(KERNEL_INC)/ -I $(KERNEL_INC)/sys/ -f elf32
KERNEL_GCC_FLAG := -I $(KERNEL_INC)/ -I $(KERNEL_INC)/sys/ -c -m32 -std=c99 -fno-builtin -nostdlib -Wall
KERNEL_LD_FLAG := -s -m elf_i386 -Ttext $(ENTRYPOINT)  -Map $(OUT_DIR)/krnl.map
KERNEL_BIN := $(OUT_DIR)/kernel.bin
KERNEL_OBJ := $(OUT_KERNEL_DIR)/kernel.o \
              $(OUT_KERNEL_DIR)/start.o \
              $(OUT_LIB_DIR)/kliba.o \
              $(OUT_LIB_DIR)/string.o \
              $(OUT_KERNEL_DIR)/i8259.o \
              $(OUT_KERNEL_DIR)/protect.o \
              $(OUT_KERNEL_DIR)/global.o \
              $(OUT_LIB_DIR)/klib.o \
              $(OUT_KERNEL_DIR)/main.o \
              $(OUT_KERNEL_DIR)/clock.o \
              $(OUT_KERNEL_DIR)/proc.o \
              $(OUT_LIB_DIR)/syscall.o \
              $(OUT_KERNEL_DIR)/keyboard.o \
              $(OUT_KERNEL_DIR)/tty.o \
              $(OUT_KERNEL_DIR)/console.o \
              $(OUT_LIB_DIR)/vsprintf.o \
              $(OUT_LIB_DIR)/printf.o \
              $(OUT_LIB_DIR)/misc.o \
              $(OUT_KERNEL_DIR)/systask.o \
              $(OUT_KERNEL_DIR)/hd.o \
              $(OUT_FS_DIR)/main.o \
              $(OUT_LIB_DIR)/open.o \
              $(OUT_LIB_DIR)/close.o \
              $(OUT_FS_DIR)/open.o \
              $(OUT_FS_DIR)/misc.o \
              $(OUT_FS_DIR)/read_write.o \
              $(OUT_LIB_DIR)/write.o \
              $(OUT_LIB_DIR)/read.o \
              $(OUT_FS_DIR)/link.o \
              $(OUT_LIB_DIR)/unlink.o \
              $(OUT_MM_DIR)/main.o \
              $(OUT_LIB_DIR)/fork.o \
              $(OUT_LIB_DIR)/getpid.o \
              $(OUT_MM_DIR)/forkexit.o \
              $(OUT_LIB_DIR)/wait.o \
              $(OUT_LIB_DIR)/exit.o

AR_FLAG := rcs
RUNTIME_LIB := $(OUT_LIB_DIR)/qhcrt.a

.PHONY: everytning all init clean install install_boot install_loader install_kernel commands

everytning: init $(BOOT_BIN) $(LOADER_BIN) $(KERNEL_BIN) $(RUNTIME_LIB)
	@echo

all: clean everytning install
	@echo

init:
	@echo QH Operating System
	@echo
	$(MKDIR_OUT)
	$(MKDIR_OUT_KERNEL)
	$(MKDIR_OUT_FS)
	$(MKDIR_OUT_LIB)
	$(MKDIR_OUT_MM)

install: install_boot install_loader install_kernel

clean:
	@echo clean...
	@echo
	@-rm -rf $(OUT_DIR)/

install_boot: $(BOOT_BIN)
	$(SHOW_INSTAL) $<
	@$(dd if=$^ of=$(IMG_FILE) bs=512 count=1 conv=notrunc)

install_loader: $(LOADER_BIN)
	$(SHOW_INSTAL) $<
	@sudo mount -o loop $(IMG_FILE) $(IMG_MOUNT)
	@sudo cp $^ $(IMG_MOUNT)
	@sudo umount $(IMG_MOUNT)

install_kernel: $(KERNEL_BIN)
	$(SHOW_INSTAL) $<
	@sudo mount -o loop $(IMG_FILE) $(IMG_MOUNT)
	@sudo cp $^ $(IMG_MOUNT)
	@sudo umount $(IMG_MOUNT)

$(BOOT_BIN): $(BOOT_SRC) $(BOOT_HDR)
	$(SHOW_ASMCPL) $@
	@$(ASM) -I $(BOOT_INC)/ -o $@ $<

$(LOADER_BIN): $(LOADER_SRC) $(LOADER_HDR)
	$(SHOW_ASMCPL) $@
	@$(ASM) -I $(BOOT_INC)/ -o $@ $<

$(KERNEL_BIN): $(KERNEL_OBJ)
	$(SHOW_LDLNK) $@
	@$(LD) $(KERNEL_LD_FLAG) -o $@ $^

$(RUNTIME_LIB): out/lib/*.o
	$(SHOW_ARPL) $@
	@$(AR) $(AR_FLAG) $@ $^

commands:
	@cd command; make

$(OUT_KERNEL_DIR)/kernel.o: $(KERNEL_PATH)/kernel.asm include/sys/sconst.inc
	$(SHOW_ASMCPL) $@
	@$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_LIB_DIR)/string.o: $(LIB_PATH)/string.asm
	$(SHOW_ASMCPL) $@
	@$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_LIB_DIR)/kliba.o: $(LIB_PATH)/kliba.asm
	$(SHOW_ASMCPL) $@
	@$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/start.o: $(KERNEL_PATH)/start.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/i8259.o: $(KERNEL_PATH)/i8259.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/protect.o: $(KERNEL_PATH)/protect.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/global.o: $(KERNEL_PATH)/global.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/klib.o: $(LIB_PATH)/klib.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/main.o: $(KERNEL_PATH)/main.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/clock.o: $(KERNEL_PATH)/clock.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/proc.o: $(KERNEL_PATH)/proc.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/syscall.o: $(LIB_PATH)/syscall.asm include/sys/sconst.inc
	$(SHOW_ASMCPL) $@
	@$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/keyboard.o: $(KERNEL_PATH)/keyboard.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/tty.o: $(KERNEL_PATH)/tty.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/console.o: $(KERNEL_PATH)/console.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/vsprintf.o: $(LIB_PATH)/vsprintf.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/printf.o: $(LIB_PATH)/printf.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/misc.o: $(LIB_PATH)/misc.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/systask.o: $(KERNEL_PATH)/systask.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_KERNEL_DIR)/hd.o: $(KERNEL_PATH)/hd.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_FS_DIR)/main.o: fs/main.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/open.o: $(LIB_PATH)/open.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/close.o: $(LIB_PATH)/close.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_FS_DIR)/open.o: fs/open.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_FS_DIR)/misc.o: fs/misc.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_FS_DIR)/read_write.o: fs/read_write.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/write.o: $(LIB_PATH)/write.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/read.o: $(LIB_PATH)/read.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_FS_DIR)/link.o: fs/link.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/unlink.o: $(LIB_PATH)/unlink.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_MM_DIR)/main.o: mm/main.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/fork.o: $(LIB_PATH)/fork.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/getpid.o: $(LIB_PATH)/getpid.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_MM_DIR)/forkexit.o: mm/forkexit.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/wait.o: $(LIB_PATH)/wait.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_LIB_DIR)/exit.o: $(LIB_PATH)/exit.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<
