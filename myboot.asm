;编译方法：nasm -o myboot.bin myboot.asm
;制作镜像：dd if=myboot.bin of=boot.img bs=512 count=1 conv=notrunc

; 字符串打印启始位置
%define PrintStart(row, col) (80 * row + col) * 2

; 描述符定义
%macro Descriptor 3
    dw  %2 & 0FFFFh
    dw  %1 & 0FFFFh
    db  (%1 >> 16) & 0FFh
    db  %3 & 0FFh
    db  ((%2 >> 8) & 0Fh) | ((%3 >> 8) & 0F0h)
    db  (%1 >> 24) & 0FFh
%endmacro

; 初始化 GDT
%macro InitGDT 1
    mov word [%1 + 2], ax
    shr eax, 16
    mov byte [%1 + 4], al
    mov byte [%1 + 7], ah
%endmacro

org 07c00h
jmp LEABLE_MAIN

[BITS 16]
    GDT_HEAD: Descriptor 0, 0, 0
    GDT_CODE32: Descriptor 0, SegCode32Len - 1, 0100000010011000b
    GDT_VEDIO: Descriptor 0B8000h, 0FFFFh, 0000000010010010b

    GdtLen  equ  $ - GDT_HEAD
    GdtPtr  dw   GdtLen - 1
            dd   0

    SelectorC32  equ  GDT_CODE32 - GDT_HEAD
    SelectorVDO  equ  GDT_VEDIO - GDT_HEAD

LEABLE_MAIN:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov sp, 0100h

    mov ax, 0B800h
    mov gs, ax
    mov cx, 0FFFFh
    xor di, di
    xor ax, ax

    CLS:
        mov [gs:di], ax
        add di, 2
    loop CLS
    mov si, HELLO_MSG
    mov di, PrintStart(0, 0)
    mov ah, 0Ch
    mov cx, LenHELLO_MSG
    cld
    SHOW:
        lodsb
        mov [gs:di], ax
        add di, 2
    loop SHOW

    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, LEABLE_CODE32
    InitGDT(GDT_CODE32)

    xor eax, eax
    mov ax, ds
    shl ax, 4
    add ax, GDT_HEAD
    mov dword [GdtPtr + 2], eax
    lgdt [GdtPtr]

    cli
    in al, 92h
    or al, 00000010b
    out 92h, al

    mov eax, cr0
    or ax, 1
    mov cr0, eax

    jmp dword SelectorC32:0

    jmp $

[BITS 32]
LEABLE_CODE32:
    mov ax, SelectorVDO
    mov gs, ax
    xor edi, edi
    xor esi, esi
    mov edi, PrintStart(1, 0)
    mov ah, 0Ch
    mov ecx, LenProMod_MSG
    mov esi, ProMod_MSG

    cld
    SHOW2:
        lodsb
        mov [gs:edi], ax
        add edi, 2
    loop SHOW2

    jmp $
SegCode32Len  equ  $ - LEABLE_CODE32

[BITS 32]
HELLO_MSG: db 'Hello, Operating System!'
LenHELLO_MSG  equ  $ - HELLO_MSG
ProMod_MSG: db 'Hello, Protected Mode!'
LenProMod_MSG  equ  $ - ProMod_MSG

times 510 - ($ - $$) db 0
dw 0aa55h