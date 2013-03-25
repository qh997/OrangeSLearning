OBJECT := execute.com

.PHONY: bin install all clean

bin: $(OBJECT)

all: bin install

clean:
	rm -rf $(OBJECT)

install: $(OBJECT)
	sudo mount -o loop pm.img /mnt/floppy
	sudo cp $^ /mnt/floppy/.
	sudo umount /mnt/floppy

$(OBJECT): MemoryPaging.asm
	nasm -o $@ $^
