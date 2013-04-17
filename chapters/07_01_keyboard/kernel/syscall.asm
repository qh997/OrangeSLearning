
%include  "sconst.inc"

INT_VECTOR_SYS_CALL  equ  0x90

_NR_get_ticks  equ  0
_NR_write      equ  1

bits  32
[section .text]
    global  get_ticks
    global  write

get_ticks:
    mov    eax, _NR_get_ticks
    int    INT_VECTOR_SYS_CALL
    ret

write:
    mov    eax, _NR_write
    mov    ebx, [esp + 4]
    mov    ecx, [esp + 8]
    int    INT_VECTOR_SYS_CALL
    ret
