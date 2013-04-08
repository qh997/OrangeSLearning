%include  "pm.inc"

org    0100h
jmp    LABEL_BEGIN

; GDT
[SECTION .gdt]
    LABEL_GDT:     Descriptor       0,             0, 0
    DESC_C_32:     Descriptor       0, LenCode32 - 1, DA_32 + DA_C
    DESC_D_VEDIO:  Descriptor 0B8000h,        0ffffh, DA_DRW

    GdtLen  equ  $ - LABEL_GDT
    GdtPtr  dw   GdtLen - 1
            dd   0

    SelectorC32  equ  DESC_C_32 - LABEL_GDT
    SelectorVdo  equ  DESC_D_VEDIO - LABEL_GDT

; IDT
[SECTION .idt]
ALIGN 32
[BITS 32]
    LABEL_IDT:
    %rep  32
           Gate  SelectorC32, SpuriousHandler, 0, DA_386IGate
    %endrep
    .020h: Gate  SelectorC32, ClockHandler, 0, DA_386IGate
    %rep  95
           Gate  SelectorC32, SpuriousHandler, 0, DA_386IGate
    %endrep
    .080h: Gate  SelectorC32, UserIntHandler, 0, DA_386IGate

    IdtLen  equ  $ - LABEL_IDT
    IdtPtr  dw   IdtLen - 1
            dd   0

[SECTION .s16]
[BITS 16]
LABEL_BEGIN:
    mov    ax, cs
    mov    ds, ax
    mov    es, ax
    mov    ss, ax
    mov    sp, 0100h

    InitGDT LABEL_CODE32, DESC_C_32

    xor    eax, eax
    mov    ax, ds
    shl    eax, 4
    add    eax, LABEL_GDT
    mov    dword [GdtPtr + 2], eax

    xor    eax, eax
    mov    ax, ds
    shl    eax, 4
    add    eax, LABEL_IDT
    mov    dword [IdtPtr + 2], eax

    lgdt   [GdtPtr]

    cli

    lidt   [IdtPtr]

    in     al, 92h
    or     al, 00000010b
    out    92h, al

    mov    eax, cr0
    or     ax, 1
    mov    cr0, eax

    jmp    dword SelectorC32:0

[SECTION .s32]
[BITS 32]
LABEL_CODE32:
    mov    ax, SelectorVdo
    mov    gs, ax

    call   Init8259A
    int    080h
    sti              ; 开中断

    jmp    $

Init8259A:
    mov    al, 011h
    out    020h, al ; 主8259, ICW1
    call   io_delay
    
    out    0A0h, al ; 从8259, ICW1
    call   io_delay

    mov    al, 020h
    out    021h, al ; 主8259, ICW2
    call   io_delay

    mov    al, 028h
    out    0A1h, al ; 从8259, ICW2
    call   io_delay

    mov    al, 004h
    out    021h, al ; 主8259, ICW3
    call   io_delay

    mov    al, 002h
    out    0A1h, al ; 从8259, ICW3
    call   io_delay

    mov    al, 001h
    out    021h, al ; 主8259, ICW4
    call   io_delay

    out    0A1h, al ; 从8259, ICW4
    call   io_delay

    mov    al, 0FEh
    out    021h, al ; 主8259, OCW1
    call   io_delay

    mov    al, 0FFh
    out    0A1h, al ; 从8259, OCW1
    call   io_delay

    ret

io_delay:
    nop
    nop
    nop
    nop
    ret

_SpuriousHandler:
    SpuriousHandler  equ  _SpuriousHandler - $$
    mov    ah, 0Ch
    mov    al, '!'
    mov    [gs:((80 * 0 + 75) * 2)], ax
    iretd

_UserIntHandler:
    UserIntHandler  equ  _UserIntHandler - $$
    mov    ah, 0Ch
    mov    al, 'G'
    mov    [gs:((80 * 0 + 69) * 2)], ax
    mov    ah, 0Ch
    mov    al, 'I'
    mov    [gs:((80 * 0 + 70) * 2)], ax
    iretd

_ClockHandler:
    ClockHandler  equ  _ClockHandler - $$
    inc    byte [gs:((80 * 0 + 70) * 2)]
    mov    al, 20h
    out    20h, al
    iretd

LenCode32  equ  $ - LABEL_CODE32
