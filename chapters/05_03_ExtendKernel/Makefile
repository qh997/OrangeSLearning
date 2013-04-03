ASM := nasm
CC := gcc
LD := ld
ENTRYPOINT := 0x30400

OUT_DIR := out
MKDIR_OUT := @[ -s out ] || mkdir $(OUT_DIR)

IMG_FILE := boot.img
IMG_MOUNT := /mnt/floppy

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
KERNEL_GCC_FLAG := -I $(KERNEL_INC)/ -c -m32 -fno-builtin
KERNEL_LD_FLAG := -s -m elf_i386 -Ttext $(ENTRYPOINT) 
KERNEL_OBJ := $(OUT_DIR)/kernel.o \
              $(OUT_DIR)/start.o \
              $(OUT_DIR)/kliba.o \
              $(OUT_DIR)/string.o
KERNEL_BIN := $(OUT_DIR)/kernel.bin

OBJ_BOOT := boot.bin
OBJ_LOADER := loader.bin
OBJ_KERNEL := kernel.bin
OBJECTS :=
OBJECTS += $(OBJ_BOOT)
OBJECTS += $(OBJ_LOADER)
OBJECTS += $(OBJ_KERNEL)

.PHONY: all init clean install install_boot install_loader install_kernel

all: init $(BOOT_BIN) $(LOADER_BIN) $(KERNEL_BIN)

init:
	@echo qh Operating System
	@echo
	$(MKDIR_OUT)

install: all install_boot install_loader install_kernel

clean:
	-rm -rf $(OUT_DIR)/

install_boot: $(BOOT_BIN)
	@echo
	@echo Install boot
	dd if=$^ of=$(IMG_FILE) bs=512 count=1 conv=notrunc
	@echo

install_loader: $(LOADER_BIN)
	@echo
	@echo Install loader
	sudo mount -o loop $(IMG_FILE) $(IMG_MOUNT)
	sudo cp $^ $(IMG_MOUNT)
	sudo umount $(IMG_MOUNT)
	@echo

install_kernel: $(KERNEL_BIN)
	@echo
	@echo Install kernel
	sudo mount -o loop $(IMG_FILE) $(IMG_MOUNT)
	sudo cp $^ /mnt/floppy/.
	sudo umount /mnt/floppy
	@echo

$(BOOT_BIN): $(BOOT_SRC) $(BOOT_HDR)
	$(ASM) -I $(BOOT_INC)/ -o $@ $<

$(LOADER_BIN): $(LOADER_SRC) $(LOADER_HDR)
	$(ASM) -I $(BOOT_INC)/ -o $@ $<

$(KERNEL_BIN): $(KERNEL_OBJ)
	$(LD) $(KERNEL_LD_FLAG) -o $@ $^

$(OUT_DIR)/kernel.o: $(KERNEL_PATH)/kernel.asm
	$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_DIR)/string.o: $(LIB_PATH)/string.asm
	$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_DIR)/kliba.o: $(LIB_PATH)/kliba.asm
	$(ASM) $(KERNEL_ASM_FLAG) -o $@ $<

$(OUT_DIR)/start.o: $(KERNEL_PATH)/start.c
	$(CC) $(KERNEL_GCC_FLAG) -o $@ $<
