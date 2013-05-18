
%include  "sys/sconst.inc"

INT_VECTOR_SYS_CALL  equ  0x90

_NR_printx  equ  0
_NR_sendrec equ  1

bits  32
[section .text]
    global  printx
    global  sendrec

printx:
    mov    eax, _NR_printx
    mov    ebx, [esp + 4]
    int    INT_VECTOR_SYS_CALL
    ret

sendrec:
    mov    eax, _NR_sendrec
    mov    ebx, [esp + 4]
    mov    ecx, [esp + 8]
    mov    edx, [esp + 12]
    int    INT_VECTOR_SYS_CALL
    ret
