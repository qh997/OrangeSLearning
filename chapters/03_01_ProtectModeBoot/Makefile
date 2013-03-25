OBJECT := pmBoot.bin

.PHONY: bin install all clean

bin: $(OBJECT)

all: bin install

clean:
	rm -rf $(OBJECT)

install:
	dd if=$(OBJECT) of=boot.img bs=512 count=1 conv=notrunc

$(OBJECT): pmBoot.asm
	nasm -o $@ $^
