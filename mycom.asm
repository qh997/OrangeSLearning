; 编译方法：nasm -o mycom.com mycom.asm
; 复制文件：./copy_com.sh

; 字符串打印启始位置
%define DispStart(row, col) (80 * row + col) * 2

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

org 0100h
jmp LEABLE_BEGIN

; GDT
[SECTION .gdt]
LEABLE_GDT:        Descriptor        0,             0,     0
DESCRIPTOR_NORMAL: Descriptor        0,        0ffffh, 0092h
DESCRIPTOR_CODE32: Descriptor        0, Code32Len - 1, 4098h
DESCRIPTOR_CODE16: Descriptor        0,        0ffffh, 0098h
DESCRIPTOR_DATA:   Descriptor        0,   DataLen - 1, 0092h
DESCRIPTOR_STACK:  Descriptor        0,    TopOfStack, 4093h
DESCRIPTOR_5MB:    Descriptor 0500000h,        0ffffh, 0092h
DESCRIPTOR_VEDIO:  Descriptor  0B8000h,        0FFFFh, 0092h

GdtLen  equ  $ - LEABLE_GDT
GdtPtr: dw   GdtLen - 1
        dd   0

; 选择子
SelectorNML  equ  DESCRIPTOR_NORMAL - LEABLE_GDT
SelectorC32  equ  DESCRIPTOR_CODE32 - LEABLE_GDT
SelectorC16  equ  DESCRIPTOR_CODE16 - LEABLE_GDT
SelectorDAT  equ  DESCRIPTOR_DATA   - LEABLE_GDT
SelectorSTK  equ  DESCRIPTOR_STACK  - LEABLE_GDT
Selector5MB  equ  DESCRIPTOR_5MB    - LEABLE_GDT
SelectorVDO  equ  DESCRIPTOR_VEDIO  - LEABLE_GDT

; 数据
[SECTION .data1]
ALIGN 32
[BITS 32]
LEABLE_DATA:
    spValueReal:   dw    0
    HelloMSG:      db    "Hello, Operating System!"
    LenHelloMSG    equ   $ - HelloMSG
    ProModMSG:     db    "Hello, Protected Mode!", 0
    OffsetModMSG   equ   ProModMSG - $$
    StrTest:       db    "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0
    OfffsetStrT    equ   StrTest - $$
    DataLen        equ   $ - LEABLE_DATA

[SECTION .gs]
ALIGN 32
[BITS 32]
LEABLE_STACK:
    times  512  db  0
TopOfStack  equ  $ - LEABLE_STACK - 1

[SECTION .s16]
[BITS 16]
LEABLE_BEGIN:
    mov    ax, cs
    mov    ds, ax
    mov    es, ax
    mov    ss, ax
    mov    sp, 0100h
    mov    [LEABLE_BACK_REAL + 3], ax
    mov    [spValueReal], sp

    mov    ax, 0B800h
    mov    gs, ax
    mov    cx, 0FFFFh
    xor    di, di
    xor    ax, ax

    mov    si, HelloMSG
    mov    di, DispStart(0, 0)
    mov    ah, 0Ah
    mov    cx, LenHelloMSG
    cld
    SHOW:
        lodsb
        mov   [gs:di], ax
        add   di, 2
    loop   SHOW

    InitGDT LEABLE_CODE32, DESCRIPTOR_CODE32
    InitGDT LEABLE_CODE16, DESCRIPTOR_CODE16
    InitGDT LEABLE_DATA, DESCRIPTOR_DATA
    InitGDT LEABLE_STACK, DESCRIPTOR_STACK

    xor    eax, eax
    mov    ax, ds
    shl    eax, 4
    add    eax, LEABLE_GDT
    mov    dword [GdtPtr + 2], eax
    lgdt   [GdtPtr]

    cli

    in     al, 92h
    or     al, 00000010b
    out    92h, al

    mov    eax, cr0
    or     eax, 1
    mov    cr0, eax

    jmp    dword SelectorC32:0

LEABLE_REAL_RETURN:
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
    mov    edi, DispStart(4, 0)
    mov    ah, 0Ah
    mov    al, 'R'
    mov    [gs:edi], ax

    xor    eax, eax
    mov    ax, 4c00h
    int    21h

[SECTION .s32]
[BITS 32]
LEABLE_CODE32:
    mov    ax, SelectorDAT
    mov    ds, ax
    mov    ax, Selector5MB
    mov    es, ax
    mov    ax, SelectorVDO
    mov    gs, ax
    mov    ax, SelectorSTK
    mov    ss, ax
    mov    esp, TopOfStack

    xor    edi, edi
    mov    edi, DispStart(1, 0)

    xor    esi, esi
    mov    esi, OffsetModMSG
    mov    ah, 0Ch
    cld
    SHOW2.1:
        lodsb
        test   al, al
        jz     SHOW2.2
        mov    [gs:edi], ax
        add    edi, 2
        jmp    SHOW2.1
    SHOW2.2:

    call   DispReturn
    call   Read5MB

    call   Write5MB

    mov    al, 7
    mov    [es:4], al

    call   Read5MB

    jmp    SelectorC16:0

Read5MB:
    push   esi
    xor    esi, esi
    mov    ecx, 8
    .loop:
        mov    al, [es:esi]
        call   DispAL
        inc    esi
        loop   .loop
    call   DispReturn
    pop   esi
    ret

Write5MB:
    push   esi
    push   edi
    xor    esi, esi
    xor    edi, edi
    mov    esi, OfffsetStrT
    cld
    .1:
        lodsb
        test   al, al
        jz     .2
        mov    [es:edi], al
        inc    edi
        jmp    .1
        .2:
    pop   edi
    pop   esi
    ret

DispAL:
    push   ecx
    push   edx
    mov    ah, 0Ch
    mov    dl, al
    shr    al, 4
    mov    ecx, 2
    .begin:
        and    al, 0Fh
        cmp    al, 9
        ja     .1
        add    al, '0'
        jmp    .2
    .1:
        sub    al, 0Ah
        add    al, 'A'
    .2:
        mov    [gs:edi], ax
        add    edi, 2

        mov    al, dl
        loop   .begin
    add    edi, 2
    pop    edx
    pop    ecx
    ret

DispReturn:
    push   eax
    push   ebx
    mov    eax, edi
    mov    bl, 160
    div    bl
    and    eax, 0FFh
    inc    eax
    mov    bl, 160
    mul    bl
    mov    edi, eax
    pop    ebx
    pop    eax
    ret

Code32Len  equ  $ - LEABLE_CODE32

[SECTION .s16code]
ALIGN 32
[BITS 16]
LEABLE_CODE16:
    mov    ax, SelectorNML
    mov    ds, ax
    mov    es, ax
    mov    fs, ax
    mov    gs, ax
    mov    ss, ax

    mov    eax, cr0
    and    al, 11111110b
    mov    cr0, eax

LEABLE_BACK_REAL:
    jmp    0:LEABLE_REAL_RETURN
