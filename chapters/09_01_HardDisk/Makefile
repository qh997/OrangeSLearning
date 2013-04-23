ASM := nasm
CC := gcc
LD := ld

SHOW_ASMCPL := @echo 'ASM '
SHOW_CCCPL  := @echo ' CC '
SHOW_LDLNK  := @echo ' LD '

ENTRYPOINT := 0x30400

OUT_DIR := out
MKDIR_OUT := @mkdir -p $(OUT_DIR)

IMG_FILE := boot.img
IMG_MOUNT := /mnt/floppy

HEADS := include/console.h \
         include/const.h \
         include/global.h \
         include/keyboard.h \
         include/keymap.h \
         include/proc.h \
         include/protect.h \
         include/proto.h \
         include/string.h \
         include/tty.h \
         include/type.h

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

KERNEL_PATH := kernel
LIB_PATH := lib
KERNEL_INC := include
KERNEL_ASM_FLAG := -I $(KERNEL_INC)/ -f elf32
KERNEL_GCC_FLAG := -I $(KERNEL_INC)/ -c -m32 -std=c99 -fno-builtin -fno-stack-protector
KERNEL_LD_FLAG := -s -m elf_i386 -Ttext $(ENTRYPOINT)
KERNEL_BIN := $(OUT_DIR)/kernel.bin
KERNEL_OBJ := $(OUT_DIR)/kernel.o \
              $(OUT_DIR)/start.o \
              $(OUT_DIR)/kliba.o \
              $(OUT_DIR)/string.o \
              $(OUT_DIR)/i8259.o \
              $(OUT_DIR)/protect.o \
              $(OUT_DIR)/global.o \
              $(OUT_DIR)/klib.o \
              $(OUT_DIR)/kmain.o \
              $(OUT_DIR)/clock.o \
              $(OUT_DIR)/proc.o \
              $(OUT_DIR)/syscall.o \
              $(OUT_DIR)/keyboard.o \
              $(OUT_DIR)/tty.o \
              $(OUT_DIR)/console.o \
              $(OUT_DIR)/vsprintf.o \
              $(OUT_DIR)/printf.o \
              $(OUT_DIR)/misc.o \
              $(OUT_DIR)/systask.o \
              $(OUT_DIR)/hd.o \
              $(OUT_DIR)/fs_main.o

OBJ_BOOT := boot.bin
OBJ_LOADER := loader.bin
OBJ_KERNEL := kernel.bin
OBJECTS :=
OBJECTS += $(OBJ_BOOT)
OBJECTS += $(OBJ_LOADER)
OBJECTS += $(OBJ_KERNEL)

.PHONY: everytning all init clean install install_boot install_loader install_kernel

everytning: init $(BOOT_BIN) $(LOADER_BIN) $(KERNEL_BIN)

all: clean everytning install

init:
	@echo QH Operating System
	@echo
	$(MKDIR_OUT)

install: install_boot install_loader install_kernel

clean:
	@echo clean...
	@echo
	@-rm -rf $(OUT_DIR)/

install_boot: $(BOOT_BIN)
	@echo Install boot
	@$(dd if=$^ of=$(IMG_FILE) bs=512 count=1 conv=notrunc)

install_loader: $(LOADER_BIN)
	@echo Install loader
	@sudo mount -o loop $(IMG_FILE) $(IMG_MOUNT)
	@sudo cp $^ $(IMG_MOUNT)
	@sudo umount $(IMG_MOUNT)

install_kernel: $(KERNEL_BIN)
	@echo Install kernel
	@sudo mount -o loop $(IMG_FILE) $(IMG_MOUNT)
	@sudo cp $^ /mnt/floppy/.
	@sudo umount /mnt/floppy

$(BOOT_BIN): $(BOOT_SRC) $(BOOT_HDR)
	$(SHOW_ASMCPL) $@
	@$(ASM) -I $(BOOT_INC)/ -o $@ $<

$(LOADER_BIN): $(LOADER_SRC) $(LOADER_HDR)
	$(SHOW_ASMCPL) $@
	@$(ASM) -I $(BOOT_INC)/ -o $@ $<

$(KERNEL_BIN): $(KERNEL_OBJ)
	$(SHOW_LDLNK) $@
	@$(LD) $(KERNEL_LD_FLAG) -o $@ $^

$(OUT_DIR)/kernel.o: $(KERNEL_PATH)/kernel.asm $(HEADS)
	$(SHOW_ASMCPL) $@
	@$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_DIR)/string.o: $(LIB_PATH)/string.asm
	$(SHOW_ASMCPL) $@
	@$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_DIR)/kliba.o: $(LIB_PATH)/kliba.asm
	$(SHOW_ASMCPL) $@
	@$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_DIR)/start.o: $(KERNEL_PATH)/start.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/i8259.o: $(KERNEL_PATH)/i8259.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/protect.o: $(KERNEL_PATH)/protect.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/global.o: $(KERNEL_PATH)/global.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/klib.o: $(LIB_PATH)/klib.c
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/kmain.o: $(KERNEL_PATH)/kmain.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/clock.o: $(KERNEL_PATH)/clock.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/proc.o: $(KERNEL_PATH)/proc.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/syscall.o: $(KERNEL_PATH)/syscall.asm $(HEADS)
	$(SHOW_ASMCPL) $@
	@$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_DIR)/keyboard.o: $(KERNEL_PATH)/keyboard.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/tty.o: $(KERNEL_PATH)/tty.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/console.o: $(KERNEL_PATH)/console.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/vsprintf.o: $(KERNEL_PATH)/vsprintf.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/printf.o: $(KERNEL_PATH)/printf.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/misc.o: $(LIB_PATH)/misc.c
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/systask.o: $(KERNEL_PATH)/systask.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/hd.o: $(KERNEL_PATH)/hd.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<

$(OUT_DIR)/fs_main.o: fs/fs_main.c $(HEADS)
	$(SHOW_CCCPL) $@
	@$(CC) $(KERNEL_GCC_FLAG) -o $@ $<
