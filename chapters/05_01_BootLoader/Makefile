OBJ_BOOT := boot.bin
OBJ_LOADER := loader.bin
OBJ_KERNEL := kernel.bin
OBJECTS :=
OBJECTS += $(OBJ_BOOT)
OBJECTS += $(OBJ_LOADER)
OBJECTS += $(OBJ_KERNEL)

.PHONY: bin install clean install_boot install_loader install_kernel

bin: $(OBJECTS)

install: install_boot install_loader install_kernel

clean:
	-rm $(OBJECTS)

$(OBJ_BOOT): OSboot.asm
	@echo
	@echo Compile boot
	nasm -o $@ $^

install_boot: $(OBJ_BOOT)
	@echo
	@echo Install boot
	dd if=$^ of=boot.img bs=512 count=1 conv=notrunc

$(OBJ_LOADER): OSloader.asm
	@echo
	@echo Compile loader
	nasm -o $@ $^

install_loader: $(OBJ_LOADER)
	@echo
	@echo Install loader
	sudo mount -o loop boot.img /mnt/floppy
	sudo cp $^ /mnt/floppy/.
	sudo umount /mnt/floppy

$(OBJ_KERNEL): OSkernel.asm
	@echo
	@echo Compile kernel
	nasm -f elf32 -o kernel.o $^
	ld -s -m elf_i386 -o $@ kernel.o
	-rm kernel.o

install_kernel: $(OBJ_KERNEL)
	@echo
	@echo Install kernel
	sudo mount -o loop boot.img /mnt/floppy
	sudo cp $^ /mnt/floppy/.
	sudo umount /mnt/floppy
