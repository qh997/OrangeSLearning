%include    "MemoryPaging_gen.inc"

PageDirBase0  equ  200000h
PageTblBase0  equ  201000h
PageDirBase1  equ  210000h
PageTblBase1  equ  211000h

LinearAddrDemo  equ  00401000h
ProcFoo         equ  00401000h
ProcBar         equ  00501000h
ProcPagingDemo  equ  00301000h

org 0100h
jmp LEABLE_BEGIN

; GDT
[SECTION .gdt]
    LEABLE_GDT:      Descriptor            0,                0,   0
    DESC_D_NORMAL:   Descriptor            0,           0ffffh,   DA_DRW
    DESC_FLAT_C:     Descriptor            0,          0fffffh,   DA_32|DA_4K|DA_CR
    DESC_FLAT_RW:    Descriptor            0,          0fffffh,   DA_4K|DA_DRW
    DESC_C_32:       Descriptor            0,    LenCode32 - 1,   DA_CR|DA_32
    DESC_C_16:       Descriptor            0,           0ffffh,   DA_C
    DESC_D_DATA:     Descriptor            0,      LenData - 1,   DA_DRW
    DESC_D_STACK:    Descriptor            0,         TopStack,   DA_32|DA_DRWA
    DESC_D_VIDEO:    Descriptor      0B8000h,           0FFFFh,   DA_DRW

    LenGDT  equ  $ - LEABLE_GDT
    GdtPtr: dw   LenGDT - 1
            dd   0

    ; 选择子
    SelectorNormal    equ  DESC_D_NORMAL  - LEABLE_GDT
    SelectorFlatC     equ  DESC_FLAT_C    - LEABLE_GDT
    SelectorFlatRW    equ  DESC_FLAT_RW   - LEABLE_GDT
    SelectorCode32    equ  DESC_C_32      - LEABLE_GDT
    SelectorCode16    equ  DESC_C_16      - LEABLE_GDT
    SelectorData      equ  DESC_D_DATA    - LEABLE_GDT
    SelectorStack     equ  DESC_D_STACK   - LEABLE_GDT
    SelectorVideo     equ  DESC_D_VIDEO   - LEABLE_GDT

; 数据
[SECTION .data1]
ALIGN 32
[BITS 32]
LEABLE_DATA:
    _dwRealsp:       dw     0
    _dwDispPos:      dd     0

    _szHello:        db     "Hello, Operating System!", 0
    _szProMod:       db     "Hello, Protected Mode!", 0Ah, 0
    _szMemChkIdx:    db     "BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
    _szRAMSize:      db     "RAM size: ", 0
    _szSpace:        db     " ", 0
    _szReturn:       db     0Ah, 0

    _dwMCRNumber:    dd     0
    _dwMemSize:      dd     0
    _ARDStruct:
        _dwBaseAddrLow:     dd    0
        _dwBaseAddrHigh:    dd    0
        _dwLengthLow:       dd    0
        _dwLengthHigh:      dd    0
        _dwType:            dd    0
    _PageTableNumber        dd    0
    _MemChkBuf:      times  256  db  0

    dwDispPos        equ    _dwDispPos - $$
    szProMod         equ    _szProMod - $$
    szMemChkIdx      equ    _szMemChkIdx - $$
    szRAMSize        equ    _szRAMSize - $$
    szSpace          equ    _szSpace - $$
    szReturn         equ    _szReturn - $$

    dwMCRNumber      equ    _dwMCRNumber - $$
    dwMemSize        equ    _dwMemSize - $$
    ARDStruct        equ    _ARDStruct - $$
        dwBaseAddrLow      equ    _dwBaseAddrLow - $$
        dwBaseAddrHigh     equ    _dwBaseAddrHigh - $$
        dwLengthLow        equ    _dwLengthLow - $$
        dwLengthHigh       equ    _dwLengthHigh - $$
        dwType             equ    _dwType - $$
    PageTableNumber        equ    _PageTableNumber - $$
    MemChkBuf        equ    _MemChkBuf - $$

    LenData  equ  $ - LEABLE_DATA

; 堆栈
[SECTION .gs]
ALIGN 32
[BITS 32]
LEABLE_STACK:
    times  512  db  0
    TopStack  equ  $ - LEABLE_STACK - 1

[SECTION .s16]
[BITS 16]
LEABLE_BEGIN:
    mov    ax, cs
    mov    ds, ax
    mov    es, ax
    mov    ss, ax
    mov    sp, 0100h
    mov    [LEABLE_BACK_REAL + 3], ax
    mov    [_dwRealsp], sp

    mov    ax, 0B800h
    mov    gs, ax
    mov    esi, _szHello
    mov    edi, [_dwDispPos]
    mov    ah, 0Ah
    cld
    .1:
        lodsb
        test   al, al
        jz     .2
        mov    [gs:edi], ax
        add    edi, 2
        jmp    .1
    .2:
    mov    [_dwDispPos], edi

    mov    ebx, 0
    mov    di, _MemChkBuf
    .mcloop:
        mov    eax, 0E820h
        mov    ecx, 20
        mov    edx, 0534D4150h
        int    15h
        jc     .mcfail
        add    di, 20
        inc    dword [_dwMCRNumber]
        cmp    ebx, 0
        jne    .mcloop
        jmp    .mcok
    .mcfail:
        mov    dword [_dwMCRNumber], 0
    .mcok:

    InitGDT LEABLE_CODE32, DESC_C_32
    InitGDT LEABLE_CODE16, DESC_C_16
    InitGDT LEABLE_DATA, DESC_D_DATA
    InitGDT LEABLE_STACK, DESC_D_STACK

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

    mov    sp, [_dwRealsp]

    in     al, 92h
    and    al, 11111101b
    out    92h, al

    sti

    xor    eax, eax
    mov    ax, 0B800h
    mov    gs, ax
    mov    edi, [_dwDispPos]
    mov    ah, 0Ah
    mov    al, 'R'
    mov    [gs:edi], ax

    xor    eax, eax
    mov    ax, 4c00h
    int    21h

[SECTION .s32]
[BITS 32]
LEABLE_CODE32:
    mov    ax, SelectorData
    mov    ds, ax
    mov    es, ax
    mov    ax, SelectorVideo
    mov    gs, ax
    mov    ax, SelectorStack
    mov    ss, ax
    mov    esp, TopStack

    call   DispReturn

    push   szProMod
    call   DispStr
    add    esp, 4
    call   DispReturn

    push   szMemChkIdx
    call   DispStr
    add    esp, 4

    call   DispMemInfo
    call   DispReturn
    call   PagingDemo

    jmp    SelectorCode16:0

    DispMemInfo:
        push   esi
        push   edi
        push   ecx

        mov    esi, MemChkBuf
        mov    ecx, [dwMCRNumber]
        .loop:
            mov    edx, 5
            mov    edi, ARDStruct
            .1:
                push   dword [esi]
                call   DispInt
                call   DispSpace
                pop    eax
                stosd
                add    esi, 4
                dec    edx
                cmp    edx, 0
                jnz    .1
                call   DispReturn
                cmp    dword [dwType], 1
                jne    .2
                mov    eax, [dwBaseAddrLow]
                add    eax, [dwLengthLow]
                cmp    eax, [dwMemSize]
                jb     .2
                mov    [dwMemSize], eax
            .2:
                loop    .loop

        call   DispReturn
        push   szRAMSize
        call   DispStr
        add    esp, 4

        push   dword [dwMemSize]
        call   DispInt
        add    esp, 4

        pop    ecx
        pop    edi
        pop    esi
        ret

    PagingDemo:
        mov    ax, cs
        mov    ds, ax
        mov    ax, SelectorFlatRW
        mov    es, ax

        push   LenFoo
        push   OffsetFoo
        push   ProcFoo
        call   MemCpy
        add    esp, 12

        push   LenBar
        push   OffsetBar
        push   ProcBar
        call   MemCpy
        add    esp, 12

        push   LenPagingDemoAll
        push   OffsetPagingDemoProc
        push   ProcPagingDemo
        call   MemCpy
        add    esp, 12

        mov    ax, SelectorData
        mov    es, ax
        mov    ds, ax

        call   SetupPaging
        call   SelectorFlatC:ProcPagingDemo
        call   PSwitch
        call   SelectorFlatC:ProcPagingDemo

        ret

    SetupPaging:
        xor    edx, edx
        mov    eax, [dwMemSize]
        mov    ebx, 400000h
        div    ebx
        mov    ecx, eax
        test   edx, edx
        jz     .no_remainder
        inc    ecx

        .no_remainder:
        mov    [PageTableNumber], ecx
        mov    ax, SelectorFlatRW
        mov    es, ax
        mov    edi, PageDirBase0
        xor    eax, eax
        mov    eax, PageTblBase0 | PG_P | PG_USU | PG_RWW
        .1:
            stosd
            add    eax, 4096
            loop   .1
        mov    eax, [PageTableNumber]
        mov    ebx, 1024
        mul    ebx
        mov    ecx, eax
        mov    edi, PageTblBase0
        xor    eax, eax
        mov    eax, PG_P | PG_USU | PG_RWW
        .2:
            stosd
            add    eax, 4096
            loop   .2

        mov    eax, PageDirBase0
        mov    cr3, eax
        mov    eax, cr0
        or     eax, 80000000h
        mov    cr0, eax
        jmp    short .3
        .3:
        nop

        ret

    PSwitch:
        mov    ax, SelectorFlatRW
        mov    es, ax
        mov    edi, PageDirBase1
        xor    eax, eax
        mov    eax, PageTblBase1 | PG_P | PG_USU | PG_RWW
        mov    ecx, [PageTableNumber]
        .1:
            stosd
            add    eax, 4096
            loop   .1
        mov    eax, [PageTableNumber]
        mov    ebx, 1024
        mul    ebx
        mov    ecx, eax
        mov    edi, PageTblBase1
        xor    eax, eax
        mov    eax, PG_P | PG_USU | PG_RWW
        .2:
            stosd
            add    eax, 4096
            loop   .2

        mov    eax, LinearAddrDemo
        shr    eax, 22
        mov    ebx, 4096
        mul    ebx
        mov    ecx, eax
        mov    eax, LinearAddrDemo
        shr    eax, 12
        and    eax, 03FFh
        mov    ebx, 4
        mul    ebx
        add    eax, ecx
        add    eax, PageTblBase1
        mov    dword [es:eax], ProcBar | PG_P | PG_USU | PG_RWW

        mov    eax, PageDirBase1
        mov    cr3, eax
        jmp    short .3
        .3:
        nop

        ret

    PagingDemoProc:
        OffsetPagingDemoProc  equ  PagingDemoProc - $$
        mov    eax, LinearAddrDemo
        call   eax
        retf
        LenPagingDemoAll  equ  $ - PagingDemoProc

    foo:
        OffsetFoo  equ  foo - $$
        mov    ah, 0Bh
        mov    al, 'F'
        mov    [gs:((80 * 17 + 0) * 2)], ax
        mov    al, 'o'
        mov    [gs:((80 * 17 + 1) * 2)], ax
        mov    [gs:((80 * 17 + 2) * 2)], ax
        ret
        LenFoo  equ  $ - foo

    bar:
        OffsetBar  equ  bar - $$
        mov    ah, 0Bh
        mov    al, 'B'
        mov    [gs:((80 * 17 + 3) * 2)], ax
        mov    al, 'a'
        mov    [gs:((80 * 17 + 4) * 2)], ax
        mov    al, 'r'
        mov    [gs:((80 * 17 + 5) * 2)], ax
        ret
        LenBar  equ  $ - bar

    %include    "MemoryPaging_lib.inc"

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
