
%include  "sconst.inc"

INT_VECTOR_SYS_CALL  equ  0x90

_NR_printx  equ  0
_NR_sendrec equ  1
_NR_get_ticks  equ  2
_NR_write      equ  3

bits  32
[section .text]
    global  printx
    global  sendrec
    global  _o_get_ticks
    global  write

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

_o_get_ticks:
    mov    eax, _NR_get_ticks
    int    INT_VECTOR_SYS_CALL
    ret

write:
    mov    eax, _NR_write
    mov    ebx, [esp + 4]
    mov    ecx, [esp + 8]
    int    INT_VECTOR_SYS_CALL
    ret
