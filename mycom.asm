; 编译方法：nasm -o mycom.com mycom.asm
; 复制文件：./copy_com.sh

%include    "gen.inc"

PageDirBase  equ  200000h
PageTblBase  equ  201000h

org 0100h
jmp LEABLE_BEGIN

; GDT
[SECTION .gdt]
    LEABLE_GDT:      Descriptor            0,                0,   0
    DESC_D_NORMAL:   Descriptor            0,           0ffffh,           DA_DRW
    DESC_D_PDIR:     Descriptor  PageDirBase,             4095,           DA_DRW
    DESC_D_PTBL:     Descriptor  PageTblBase,             1023,   DA_4K + DA_DRW
    DESC_C_32:       Descriptor            0,    LenCode32 - 1,   DA_32 + DA_C
    DESC_C_16:       Descriptor            0,           0ffffh,           DA_C
    DESC_C_DEST:     Descriptor            0,    LenCodeDt - 1,   DA_32 + DA_C
    DESC_C_R3:       Descriptor            0,    LenCodeR3 - 1,   DA_32 + DA_C        + DA_DPL3
    DESC_D_1:        Descriptor            0,     LenData1 - 1,           DA_DRW
    DESC_D_STACK:    Descriptor            0,         TopStack,   DA_32 + DA_DRWA
    DESC_D_STACK3:   Descriptor            0,        TopStack3,   DA_32 + DA_DRWA     + DA_DPL3
    DESC_D_5MB:      Descriptor     0500000h,           0ffffh,           DA_DRW
    DESC_L_LDT:      Descriptor            0,       LenLDT - 1,           DA_LDT
    DESC_D_VEDIO:    Descriptor      0B8000h,           0FFFFh,           DA_DRW      + DA_DPL3
    DESC_T_TSS:      Descriptor            0,       LenTSS - 1,           DA_386TSS
    DESC_G_TEST:     Gate    SelectorCodeDst,          0,    0,           DA_386CGate + DA_DPL3

    LenGDT  equ  $ - LEABLE_GDT
    GdtPtr: dw   LenGDT - 1
            dd   0

    ; 选择子
    SelectorNormal    equ  DESC_D_NORMAL  - LEABLE_GDT
    SelectorPageDir   equ  DESC_D_PDIR    - LEABLE_GDT
    SelectorPageTbl   equ  DESC_D_PTBL    - LEABLE_GDT
    SelectorCode32    equ  DESC_C_32      - LEABLE_GDT
    SelectorCode16    equ  DESC_C_16      - LEABLE_GDT
    SelectorCodeDst   equ  DESC_C_DEST    - LEABLE_GDT
    SelectorCodeR3    equ  DESC_C_R3      - LEABLE_GDT + SA_RPL3
    SelectorData1     equ  DESC_D_1       - LEABLE_GDT
    SelectorStack     equ  DESC_D_STACK   - LEABLE_GDT
    SelectorStackR3   equ  DESC_D_STACK3  - LEABLE_GDT + SA_RPL3
    Selector5MB       equ  DESC_D_5MB     - LEABLE_GDT
    SelectorLDT       equ  DESC_L_LDT     - LEABLE_GDT
    SelectorTSS       equ  DESC_T_TSS     - LEABLE_GDT
    SelectorVedio     equ  DESC_D_VEDIO   - LEABLE_GDT

    SelectorGTT  equ  DESC_G_TEST     - LEABLE_GDT + SA_RPL3

; 数据
[SECTION .data1]
ALIGN 32
[BITS 32]
LEABLE_DATA1:
    spValueReal:   dw    0
    HelloMSG:      db    "Hello, Operating System!", 0
    LenHelloMSG    equ   $ - HelloMSG

    _szHello:        db  "Hello, Operating System!", 0
    _szProMod:       db  "Hello, Protected Mode!", 0
    _szLetter:       db  "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0
    _szMemChkTitle:  db  "BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
    _szRAMSize:      db  "RAM size:", 0
    _szReturn:       db  0Ah, 0

    szProMod   equ   _szProMod - $$
    szLetter    equ   _szLetter - $$

    LenData1       equ   $ - LEABLE_DATA1

; 堆栈
[SECTION .gs]
ALIGN 32
[BITS 32]
LEABLE_STACK:
    times  512  db  0
    TopStack  equ  $ - LEABLE_STACK - 1

[SECTION .gs3]
ALIGN 32
[BITS 32]
LEABLE_STACK3:
    times  512  db  0
    TopStack3 equ  $ - LEABLE_STACK3 - 1

[SECTION .tss]
ALIGN 32
[BITS 32]
LEABLE_TSS:
    DD    0                  ; Back
    DD    TopStack           ; 0 级堆栈
    DD    SelectorStack      ;
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
    
    LenTSS  equ  $ - LEABLE_TSS

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

    PrintString 0B800h, 0, 0, 0Ah, _szHello

    InitGDT LEABLE_CODE32,    DESC_C_32
    InitGDT LEABLE_CODE16,    DESC_C_16
    InitGDT LEABLE_CODE_DEST, DESC_C_DEST
    InitGDT LEABLE_CODE_R3,   DESC_C_R3
    InitGDT LEABLE_DATA1,     DESC_D_1
    InitGDT LEABLE_STACK,     DESC_D_STACK
    InitGDT LEABLE_STACK3,    DESC_D_STACK3
    InitGDT LEABLE_LDT,       DESC_L_LDT
    InitGDT LEABLE_CODE_A,    DESC_L_CODEA
    InitGDT LEABLE_TSS,       DESC_T_TSS

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

    jmp    dword SelectorCode32:0

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
    call   SetupPaging

    mov    ax, SelectorData1
    mov    ds, ax
    mov    ax, Selector5MB
    mov    es, ax
    mov    ax, SelectorVedio
    mov    gs, ax
    mov    ax, SelectorStack
    mov    ss, ax
    mov    esp, TopStack

    PrintString SelectorVedio, 1, 0, 0Ch, szProMod

    call   DispReturn
    call   Read5MB
    call   Write5MB
    mov    al, 7
    mov    [es:4], al
    call   Read5MB

    mov    ax, SelectorTSS
    ltr    ax

    push   SelectorStackR3
    push   TopStack3
    push   SelectorCodeR3
    push   0
    retf

    call   SelectorGTT:0

    mov    ax, SelectorLDT
    lldt   ax

    jmp    SelectorLCodeA:0

    SetupPaging:
        mov    ax, SelectorPageDir
        mov    es, ax
        mov    ecx, 1024
        xor    edi, edi
        xor    eax, eax
        mov    eax, PageTblBase | PG_P  | PG_USU | PG_RWW
        .1:
            stosd
            add    eax, 4096
            loop   .1

        mov    ax, SelectorPageTbl
        mov    es, ax
        mov    ecx, 1024 * 1024
        xor    edi, edi
        xor    eax, eax
        mov    eax, PG_P | PG_USU | PG_RWW
        .2:
            stosd
            add    eax, 4096
            loop   .2

        mov    eax, PageDirBase
        mov    cr3, eax
        mov    eax, cr0
        or     eax, 80000000h
        mov    cr0, eax
        jmp    short .3
        .3:
            nop
        ret

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
        mov    esi, szLetter
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

    %include    "libs.inc"

    LenCode32  equ  $ - LEABLE_CODE32

[SECTION .s16code]
ALIGN 32
[BITS 16]
LEABLE_CODE16:
    mov    ax, SelectorNormal
    mov    ds, ax
    mov    es, ax
    mov    fs, ax
    mov    gs, ax
    mov    ss, ax

    mov    eax, cr0
    and    eax, 7FFFFFFEh
    mov    cr0, eax

    LEABLE_BACK_REAL:
        jmp    0:LEABLE_REAL_RETURN

[SECTION .ldt]
ALIGN 32
LEABLE_LDT:
    DESC_L_CODEA:  Descriptor  0, CodeALen - 1, 4098h

    LenLDT  equ  $ - LEABLE_LDT

    ; 选择子
    SelectorLCodeA  equ  DESC_L_CODEA - LEABLE_LDT + 4

[SECTION .la]
ALIGN 32
[BITS 32]
LEABLE_CODE_A:
    PrintLetter SelectorVedio, 4, 7, 0Ch, 'L'

    jmp    SelectorCode16:0

    CodeALen  equ  $ - LEABLE_CODE_A

[SECTION .sdest]
[BITS 32]
LEABLE_CODE_DEST:
    PrintLetter SelectorVedio, 4, 5, 0Ch, 'G'

    mov    ax, SelectorLDT
    lldt   ax
    jmp    SelectorLCodeA:0

    LenCodeDt  equ  $ - LEABLE_CODE_DEST

[SECTION .cr3]
[BITS 32]
LEABLE_CODE_R3:
    PrintLetter SelectorVedio, 4, 3, 0Ch, '3'

    call   SelectorGTT:0

    LenCodeR3  equ  $ - LEABLE_CODE_R3
