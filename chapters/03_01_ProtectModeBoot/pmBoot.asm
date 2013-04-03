; 字符串打印启始位置
%define PrintStart(row, col) (80 * row + col) * 2

; 描述符定义
%macro Descriptor 3
    dw  %2 & 0FFFFh
    dw  %1 & 0FFFFh
    db  (%1 >> 16) & 0FFh
    db  %3 & 0FFh
    db  ((%2 >> 16) & 0Fh) | ((%3 >> 8) & 0F0h)
    db  (%1 >> 24) & 0FFh
%endmacro

; 初始化 GDT
%macro InitGDT 2
    xor    eax, eax
    mov    ax, cs
    shl    eax, 4
    add    eax, %1
    mov    word [%2 + 2], ax
    shr    eax, 16
    mov    byte [%2 + 4], al
    mov    byte [%2 + 7], ah
%endmacro

org 07c00h
jmp LABEL_MAIN

; GDT
LABEL_GDT:         Descriptor        0,                0,     0
DESCRIPTOR_NORMAL: Descriptor        0,           0ffffh, 0092h
DESCRIPTOR_CODE32: Descriptor        0, SegCode32Len - 1, 4098h
DESCRIPTOR_CODE16: Descriptor        0,           0ffffh, 0098h
DESCRIPTOR_DATA:   Descriptor        0,      DataLen - 1, 0092h
DESCRIPTOR_5MB:    Descriptor 0500000h,           0ffffh, 0092h
DESCRIPTOR_VEDIO:  Descriptor  0B8000h,           0FFFFh, 0092h

GdtLen  equ  $ - LABEL_GDT
GdtPtr  dw   GdtLen - 1
        dd   0

; 选择子
SelectorNML  equ  DESCRIPTOR_NORMAL - LABEL_GDT
SelectorC32  equ  DESCRIPTOR_CODE32 - LABEL_GDT
SelectorC16  equ  DESCRIPTOR_CODE16 - LABEL_GDT
SelectorDAT  equ  DESCRIPTOR_DATA   - LABEL_GDT
Selector5MB  equ  DESCRIPTOR_5MB    - LABEL_GDT
SelectorVDO  equ  DESCRIPTOR_VEDIO  - LABEL_GDT

; 数据
ALIGN 32
[BITS 32]
LABEL_DATA:
    spValueReal:   dw    0
    HELLO_MSG:     db    'Hello, Operating System!!'
    LenHELLO_MSG   equ   $ - HELLO_MSG
    ProMod_MSG:    db    'Hello, Protected Mode!'
    LenProMod_MSG  equ   $ - ProMod_MSG
    DataLen        equ   $ - LABEL_DATA

[BITS 16]
LABEL_MAIN:
    mov    ax, cs
    mov    ds, ax
    mov    es, ax
    mov    ss, ax
    mov    sp, 0100h
    mov    [LABEL_REAL_RETURN + 3], ax
    mov    [spValueReal], sp

    mov    ax, 0B800h
    mov    gs, ax
    mov    cx, 0FFFFh
    xor    di, di
    xor    ax, ax

    CLS:
        mov    [gs:di], ax
        add    di, 2
    loop   CLS
    mov    si, HELLO_MSG
    mov    di, PrintStart(0, 0)
    mov    ah, 0Ah
    mov    cx, LenHELLO_MSG
    cld
    SHOW:
        lodsb
        mov   [gs:di], ax
        add   di, 2
    loop   SHOW

    InitGDT LABEL_CODE16, DESCRIPTOR_CODE16
    InitGDT LABEL_CODE32, DESCRIPTOR_CODE32
    InitGDT LABEL_DATA, DESCRIPTOR_DATA

    xor    eax, eax
    mov    ax, ds
    shl    eax, 4
    add    eax, LABEL_GDT
    mov    dword [GdtPtr + 2], eax
    lgdt   [GdtPtr]

    cli

    in     al, 92h
    or     al, 00000010b
    out    92h, al

    mov    eax, cr0
    or     ax, 1
    mov    cr0, eax

    jmp   dword SelectorC32:0

LABEL_REAL_RETURN:
    mov    ax, cs
    mov    ds, ax
    mov    es, ax
    mov    ss, ax

    mov    sp, [spValueReal]

    in     al, 92h
    and    al, 11111101b
    out    92h, al

    sti

    xor    eax, eax
    mov    ax, 0B800h
    mov    gs, ax
    mov    edi, PrintStart(3, 0)
    mov    ah, 0Ah
    mov    al, 'R'
    mov    [gs:edi], ax

    jmp    $

[BITS 32]
LABEL_CODE32:
    mov    ax, SelectorVDO
    mov    gs, ax
    xor    edi, edi
    xor    esi, esi
    mov    edi, PrintStart(1, 0)
    mov    ah, 0Ch
    mov    ecx, LenProMod_MSG
    mov    esi, ProMod_MSG

    cld
    SHOW2:
        lodsb
        mov    [gs:edi], ax
        add    edi, 2
    loop SHOW2

    mov    ax, Selector5MB
    mov    es, ax

    mov    al, [es:0]
    add    al, '0'
    mov    ah, 0Ch
    mov    edi, PrintStart(2, 0)
    mov    [gs:edi], ax

    mov    al, 7
    mov    [es:0], al

    mov    al, [es:0]
    add    al, '0'
    mov    ah, 0Ch
    mov    edi, PrintStart(2, 2)
    mov    [gs:edi], ax

    jmp    SelectorC16:0

SegCode32Len  equ  $ - LABEL_CODE32

[BITS 16]
LABEL_CODE16:
    mov    ax, SelectorNML
    mov    ds, ax
    mov    es, ax
    mov    fs, ax
    mov    gs, ax
    mov    ss, ax

    mov    eax, cr0
    and    ax, 11111110b
    mov    cr0, eax

LABEL_RETURN_REAL:
    jmp    0:LABEL_REAL_RETURN

times 510 - ($ - $$) db 0
dw 0aa55h
