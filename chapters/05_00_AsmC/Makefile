ASM := nasm
ASMFLAG := -f elf
CC := gcc
CCFLAG := -c -m32 -std=c99 -fno-builtin -fno-stack-protector
LD := ld
LDFLAG := -s -m elf_i386

.PHONY:

hello: hello.o foobar.o
	$(LD) $(LDFLAG) -o $@ $^

hello.o: hello.asm
	$(ASM) $(ASMFLAG) -o $@ $<

foobar.o: foobar.c
	$(CC) $(CCFLAG) -o $@ $<
