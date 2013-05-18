extern main
extern exit

BITS 32

[section .text]
global _start

_start:
    push   eax
    push   ecx
    call   main

    push   eax
    call   exit

    hlt
