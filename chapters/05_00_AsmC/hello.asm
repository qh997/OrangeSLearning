extern  choose

[section .data]
    num1st:    dd  3
    num2nd:    dd  4
    strHello:  db  "Hello, world!", 0Ah
    STRLEN  equ  $ - strHello

[section .text]
global  _start
global  myprint
global  myprint1

_start:
    mov    edx, STRLEN
    mov    ecx, strHello
    mov    ebx, 1
    mov    eax, 4
    int    0x80

    push   dword [num2nd]
    push   dword [num1st]
    call   choose
    add    esp, 8

    mov    ebx, 0
    mov    eax, 1
    int    0x80

myprint:
    mov    edx, [esp + 8]
    mov    ecx, [esp + 4]
    mov    ebx, 1
    mov    eax, 4
    int    0x80
    ret
