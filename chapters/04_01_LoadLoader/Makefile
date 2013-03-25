BOOT_OBJ := OSboot.bin
LOADER_OBJ := loader.bin

.PHONY: bin install all clean

bin: $(BOOT_OBJ) $(LOADER_OBJ)

all: clean bin install

clean:
	rm -rf $(BOOT_OBJ) $(LOADER_OBJ)

install:
	dd if=$(BOOT_OBJ) of=boot.img bs=512 count=1 conv=notrunc
	sudo mount -o loop boot.img /mnt/floppy
	sudo cp $(LOADER_OBJ) /mnt/floppy/.
	sudo umount /mnt/floppy

$(BOOT_OBJ): OSboot.asm
	nasm -o $@ $^

$(LOADER_OBJ): OSloader.asm
	nasm -o $@ $^
