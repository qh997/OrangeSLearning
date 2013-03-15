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

%macro Gate 4
    dw  (%2 & 0FFFFh)
    dw  %1
    dw  (%3 & 1Fh) | ((%4 << 8) & 0FF00h)
    dw  ((%2 >> 16) & 0FFFFh)
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

                        ; GD        P  S type
                        ; 0000 ---- 0000 0000
DA_32        EQU  4000h ; 0100
DA_DPL0      EQU    00h ;           0000
DA_DPL1      EQU    20h ;           0010
DA_DPL2      EQU    40h ;           0100
DA_DPL3      EQU    60h ;           0110
DA_DR        EQU    90h ;           1001 0000
DA_DRW       EQU    92h ;           1001 0010
DA_DRWA      EQU    93h ;           1001 0011
DA_C         EQU    98h ;           1001 1000
DA_CR        EQU    9Ah ;           1001 1010
DA_CCO       EQU    9Ch ;           1001 1100
DA_CCOR      EQU    9Eh ;           1001 1110
DA_LDT       EQU    82h ;           1000 0010
DA_TaskGate  EQU    85h ;           1000 0101
DA_386TSS    EQU    89h ;           1000 1001
DA_386CGate  EQU    8Ch ;           1000 1100
DA_386IGate  EQU    8Eh ;           1000 1110
DA_386TGate  EQU    8Fh ;           1000 1111

                 ; -...- TI RPL
                 ; 15..3  2 10
SA_RPL0   EQU  0 ;          00
SA_RPL1   EQU  1 ;          01
SA_RPL2   EQU  2 ;          10
SA_RPL3   EQU  3 ;          11


org 0100h
jmp LEABLE_BEGIN

; GDT
[SECTION .gdt]
    LEABLE_GDT:          Descriptor            0,             0, 0
    DESCRIPTOR_NORMAL:   Descriptor            0,        0ffffh,         DA_DRW
    DESCRIPTOR_CODE32:   Descriptor            0, Code32Len - 1, DA_32 + DA_C
    DESCRIPTOR_CODE16:   Descriptor            0,        0ffffh,         DA_C
    DESCRIPTOR_DEST:     Descriptor            0, CodeDtLen - 1, DA_32 + DA_C
    DESCRIPTOR_CODE_R3:  Descriptor            0, CodeR3Len - 1, DA_32 + DA_C        + DA_DPL3
    DESCRIPTOR_DATA:     Descriptor            0,   DataLen - 1,         DA_DRW
    DESCRIPTOR_STACK:    Descriptor            0,    TopOfStack, DA_32 + DA_DRWA
    DESCRIPTOR_STACK3:   Descriptor            0,   TopOfStack3, DA_32 + DA_DRWA     + DA_DPL3
    DESCRIPTOR_5MB:      Descriptor     0500000h,        0ffffh,         DA_DRW
    DESCRIPTOR_LDT:      Descriptor            0,    LdtLen - 1,         DA_LDT
    DESCRIPTOR_VEDIO:    Descriptor      0B8000h,        0FFFFh,         DA_DRW      + DA_DPL3
    DESCRIPTOR_TSS:      Descriptor            0,    TssLen - 1,         DA_386TSS
    DESC_GATE_TEST:      Gate        SelectorCdt,       0,    0,         DA_386CGate + DA_DPL3

    GdtLen  equ  $ - LEABLE_GDT
    GdtPtr: dw   GdtLen - 1
            dd   0

    ; 选择子
    SelectorNML  equ  DESCRIPTOR_NORMAL  - LEABLE_GDT
    SelectorC32  equ  DESCRIPTOR_CODE32  - LEABLE_GDT
    SelectorC16  equ  DESCRIPTOR_CODE16  - LEABLE_GDT
    SelectorCdt  equ  DESCRIPTOR_DEST    - LEABLE_GDT
    SelectorCR3  equ  DESCRIPTOR_CODE_R3 - LEABLE_GDT + SA_RPL3
    SelectorDAT  equ  DESCRIPTOR_DATA    - LEABLE_GDT
    SelectorSTK  equ  DESCRIPTOR_STACK   - LEABLE_GDT
    SelectorSK3  equ  DESCRIPTOR_STACK3  - LEABLE_GDT + SA_RPL3
    Selector5MB  equ  DESCRIPTOR_5MB     - LEABLE_GDT
    SelectorLDT  equ  DESCRIPTOR_LDT     - LEABLE_GDT
    SelectorTSS  equ  DESCRIPTOR_TSS     - LEABLE_GDT
    SelectorVDO  equ  DESCRIPTOR_VEDIO   - LEABLE_GDT

    SelectorGTT  equ  DESC_GATE_TEST     - LEABLE_GDT + SA_RPL3

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

; 堆栈
[SECTION .gs]
ALIGN 32
[BITS 32]
LEABLE_STACK:
    times  512  db  0
    TopOfStack  equ  $ - LEABLE_STACK - 1

[SECTION .gs3]
ALIGN 32
[BITS 32]
LEABLE_STACK3:
    times  512  db  0
    TopOfStack3 equ  $ - LEABLE_STACK3 - 1

[SECTION .tss]
ALIGN 32
[BITS 32]
LEABLE_TSS:
    DD    0                  ; Back
    DD    TopOfStack         ; 0 级堆栈
    DD    SelectorSTK        ;
    DD    0                  ; 1 级堆栈
    DD    0                  ; 
    DD    0                  ; 2 级堆栈
    DD    0                  ; 
    DD    0                  ; CR3
    DD    0                  ; EIP
    DD    0                  ; EFLAGS
    DD    0                  ; EAX
    DD    0                  ; ECX
    DD    0                  ; EDX
    DD    0                  ; EBX
    DD    0                  ; ESP
    DD    0                  ; EBP
    DD    0                  ; ESI
    DD    0                  ; EDI
    DD    0                  ; ES
    DD    0                  ; CS
    DD    0                  ; SS
    DD    0                  ; DS
    DD    0                  ; FS
    DD    0                  ; GS
    DD    0                  ; LDT
    DW    0                  ; 调试陷阱标志
    DW    $ - LEABLE_TSS + 2 ; I/O位图基址
    DB    0ffh               ; I/O位图结束标志
    
    TssLen  equ  $ - LEABLE_TSS

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

    InitGDT LEABLE_CODE32,    DESCRIPTOR_CODE32
    InitGDT LEABLE_CODE16,    DESCRIPTOR_CODE16
    InitGDT LEABLE_CODE_DEST, DESCRIPTOR_DEST
    InitGDT LEABLE_CODE_R3,   DESCRIPTOR_CODE_R3
    InitGDT LEABLE_DATA,      DESCRIPTOR_DATA
    InitGDT LEABLE_STACK,     DESCRIPTOR_STACK
    InitGDT LEABLE_STACK3,    DESCRIPTOR_STACK3
    InitGDT LEABLE_LDT,       DESCRIPTOR_LDT
    InitGDT LEABLE_CODE_A,    DESCRIPTOR_LDT_CODEA
    InitGDT LEABLE_TSS,       DESCRIPTOR_TSS

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
    mov    edi, DispStart(5, 0)
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

    mov    ax, SelectorTSS
    ltr    ax

    push   SelectorSK3
    push   TopOfStack3
    push   SelectorCR3
    push   0
    retf

    call   SelectorGTT:0

    mov    ax, SelectorLDT
    lldt   ax

    jmp    SelectorLDTCodeA:0

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

[SECTION .ldt]
ALIGN 32
LEABLE_LDT:
    DESCRIPTOR_LDT_CODEA:  Descriptor  0, CodeALen - 1, 4098h

    LdtLen  equ  $ - LEABLE_LDT

    ; 选择子
    SelectorLDTCodeA  equ  DESCRIPTOR_LDT_CODEA - LEABLE_LDT + 4

[SECTION .la]
ALIGN 32
[BITS 32]
LEABLE_CODE_A:
    mov    ax, SelectorVDO
    mov    gs, ax

    mov    edi, DispStart(4, 7)
    mov    ah, 0Ch
    mov    al, 'L'
    mov    [gs:edi], ax

    jmp    SelectorC16:0

    CodeALen  equ  $ - LEABLE_CODE_A

[SECTION .sdest]
[BITS 32]
LEABLE_CODE_DEST:
    mov    ax, SelectorVDO
    mov    gs, ax

    mov    edi, DispStart(4, 5)
    mov    ah, 0Ch
    mov    al, 'G'
    mov    [gs:edi], ax

    mov    ax, SelectorLDT
    lldt   ax

    jmp    SelectorLDTCodeA:0

    retf

    CodeDtLen  equ  $ - LEABLE_CODE_DEST

[SECTION .cr3]
[BITS 32]
LEABLE_CODE_R3:
    mov    ax, SelectorVDO
    mov    gs, ax

    mov    edi, DispStart(4, 3)
    mov    ah, 0Ch
    mov    al, '3'
    mov    [gs:edi], ax

    call   SelectorGTT:0

    jmp    $

    CodeR3Len  equ  $ - LEABLE_CODE_R3
