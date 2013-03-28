org    0100h
jmp    LABEL_START

%include  "fat12.inc"
%include  "ptmode.inc"

LABEL_GDT:           Descriptor       0,       0, 0                  ; 空描述符
LABEL_DESC_FLAT_C:   Descriptor       0, 0fffffh, DA_CR |DA_32|DA_4K ; 0-4G
LABEL_DESC_FLAT_RW:  Descriptor       0, 0fffffh, DA_DRW|DA_32|DA_4K ; 0-4G
LABEL_DESC_VIDEO:    Descriptor 0B8000h,  0ffffh, DA_DRW|DA_DPL3     ; 显存首地址

GdtLen  equ $ - LABEL_GDT
GdtPtr  dw  GdtLen - 1                      ; 段界限
        dd  BaseOfLoaderPhyAddr + LABEL_GDT ; 基地址

SelectorFlatC   equ  LABEL_DESC_FLAT_C  - LABEL_GDT
SelectorFlatRW  equ  LABEL_DESC_FLAT_RW - LABEL_GDT
SelectorVideo   equ  LABEL_DESC_VIDEO   - LABEL_GDT + SA_RPL3

BaseOfStack  equ  0100h
PageDirBase  equ  100000h ; 页目录开始地址: 1M
PageTblBase  equ  101000h ; 页表开始地址: 1M + 4K

LABEL_START:
    mov    ax, cs
    mov    ds, ax
    mov    es, ax
    mov    ss, ax
    mov    sp, BaseOfStack

    mov    dx, 0003h
    call   DispStrRM

    ; 得到内存数
    mov    ebx, 0 ; 后续值，开始时为 0
    mov    di, _MemChkBuf ; es:di 指向一个地址范围描述符结构(ARDS)
    .mcloop:
        mov    eax, 0E820h
        mov    ecx, 20
        mov    edx, 0534D4150h
        int    15h
        jc     .mcfail
        add    di, 20
        inc    dword [_dwMCRNumber] ; dwMCRNumber：ARDS 的个数
        cmp    ebx, 0
        jne    .mcloop
        jmp    .mcok
        .mcfail:
            mov    dword [_dwMCRNumber], 0
        .mcok:

    xor    ah, ah ; ┓
    xor    dl, dl ; ┣ 软驱复位
    int    13h    ; ┛

    mov    word [wSectorNo], SectorNoOfRootDir
    LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
        cmp    word [wRootDirSizeForLoop], 0 ; ┓ 根目录遍历结束
        jz     LABEL_NO_KERNELBIN            ; ┻ 没有找到 kernel.bin
        dec    word [wRootDirSizeForLoop]

        mov    ax, BaseOfKernelFile   ; ┓
        mov    es, ax                 ; ┃
        mov    bx, OffsetOfKernelFile ; ┃ es:bx = BaseOfKernelFile:OffsetOfKernelFile
        mov    ax, [wSectorNo]        ; ┃ 从第 19 个扇区开始
        mov    cl, 1                  ; ┃ 一次只读一个扇区
        call   ReadSector             ; ┻ 将第 ax 个扇区开始的 cl 个扇区读入 es:bx 中

        mov    si, KernelFileName     ; ds:si -> "KERNEL  BIN"
        mov    di, OffsetOfKernelFile ; es:di -> BaseOfKernelFile:OffsetOfKernelFile
        cld
        mov    dx, 10h ; 每个扇区一共有 512/32=16(10h) 个条目
        LABEL_SEARCH_FOR_LOADERBIN:
            cmp    dx, 0                              ; ┓ 该扇区遍历结束
            jz     LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR ; ┻ 没有找到 loader.bin
            dec    dx
            mov    cx, 11 ; "KERNEL  BIN" 长度为 11 位
            LABEL_CMP_FILENAME:
                cmp    cx, 0                ; ┓
                jz     LABEL_FILENAME_FOUND ; ┻ 找到了 loader.bin
                dec    cx
                lodsb
                cmp    al, byte [es:di]
                jz     LABEL_GO_ON
                jmp    LABEL_DIFFERENT

                LABEL_GO_ON:
                    inc    di
                    jmp    LABEL_CMP_FILENAME

                LABEL_DIFFERENT:
                    and    di, 0FFE0h ; ┓ 回到当前条目的起始位置
                    add    di, 20h    ; ┻ 指向下一个条目的起始位置
                    mov    si, KernelFileName
                    jmp    LABEL_SEARCH_FOR_LOADERBIN

            LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
                add    word [wSectorNo], 1 ; 下一个扇区
                jmp    LABEL_SEARCH_IN_ROOT_DIR_BEGIN

    LABEL_NO_KERNELBIN:
        mov    dx, 0203h
        call   DispStrRM

        %ifdef  _BOOT_DEBUG_
            mov    ax, 4c00h
            int    21h
        %else
            jmp    $
        %endif

    LABEL_FILENAME_FOUND:
        mov    ax, RootDirSectors
        and    di, 0FFE0h ; ┓ 回到当前条目的起始位置
        add    di, 01Ah   ; ┻ 指向当前条目的起始簇号（DIR_FstClus）
        mov    cx, word [es:di] ; ┓
        push   cx               ; ┻ 保存开始的簇号（在 FAT 中的序号）
        add    cx, ax            ; ┓ cx = DeltaSectorNo + RootDirSectors + 文件的起始簇号
        add    cx, DeltaSectorNo ; ┻ cx = 文件的起始扇区号
        mov    ax, BaseOfKernelFile
        mov    es, ax
        mov    bx, OffsetOfKernelFile
        mov    ax, cx ; ax = 文件的起始扇区号

    LABEL_GOON_LOADING_FILE:
        push   ax
        push   bx
        mov    ah, 0Eh ; ┓
        mov    al, '.' ; ┃
        mov    bl, 0Fh ; ┣ 每读一个扇区就打印一个点 “.”
        int    10h     ; ┛

        pop    bx          ; ┓ es:bx = BaseOfKernelFile:OffsetOfKernelFile + n * BPB_BytsPerSec
        pop    ax          ; ┃ ax = 文件当前的扇区号
        mov    cl, 1       ; ┣ 读一个扇区
        call   ReadSector  ; ┛
        pop    ax          ; 取出此扇区在 FAT 中的序号
        call   GetFATEntry ; 调用结束后 ax = 下一个簇号（在 FAT 中的序号）
        cmp    ax, 0FFFh         ; ┓
        jz     LABEL_FILE_LOADED ; ┻ 这是最后一个簇
        push   ax                   ; 保存当前簇号（在 FAT 中的序号）
        mov    dx, RootDirSectors
        add    ax, dx
        add    ax, DeltaSectorNo    ; ax = 文件当前当前簇号对应的的扇区号
        add    bx, [BPB_BytsPerSec] ; bx = 下一个扇区
        jmp    LABEL_GOON_LOADING_FILE

    LABEL_FILE_LOADED:
        call   KillMotor
        mov    dx, 0103h ; "Ready.   "
        call   DispStrRM

        lgdt   [GdtPtr]

        cli

        in     al, 92h
        or     al, 00000010b
        out    92h, al

        mov    eax, cr0
        or     eax, 1
        mov    cr0, eax

        jmp    dword SelectorFlatC:(BaseOfLoaderPhyAddr + LABEL_PM_START)

%include  "lib_rm.inc"

KillMotor:
    push   dx
    mov    dx, 03F2h
    mov    al, 0
    out    dx, al
    pop    dx
    ret

wSectorNo:            dw   0              ; 要读取的扇区号
wRootDirSizeForLoop:  dw   RootDirSectors ; 用于循环的根目录占用的扇区数, 在循环中会递减至零
bOdd:                 db   0

KernelFileName:       db   "KERNEL  BIN", 0
MessageLength         equ  9
MessageStart:         db   "Loading  "
Message1:             db   "Ready.   "
Message2:             db   "No KERNEL"

[SECTION .s32]
ALIGN 32
[BITS 32]
LABEL_PM_START:
    mov    ax, SelectorVideo
    mov    gs, ax

    mov    ax, SelectorFlatRW
    mov    ds, ax
    mov    es, ax
    mov    fs, ax
    mov    ss, ax
    mov    esp, TopOfStack

    push   szMemChkTitle
    call   DispStr
    add    esp, 4

    call   DispMemInfo
    call   SetupPaging

    mov    ah, 0Fh
    mov    al, 'P'
    mov    [gs:((80 * 0 + 39) * 2)], ax

    call   InitKernel
    jmp    SelectorFlatC:KernelEntryPointPhyAddr

DispMemInfo:
    push   esi
    push   edi
    push   ecx

    mov    esi, MemChkBuf
    mov    ecx, [dwMCRNumber]       ; for (i=0; i<[MCRNumber]; i++)
    .loop:                          ; {
        mov    edx, 5               ;     for (j=0; j<5; j++)
        mov    edi, ARDStruct       ;     {
        .1:                         ;
            push   dword [esi]      ;
            call   DispInt          ;         DispInt(MemChkBuf[j*4]); // 显示一个成员
            call   DispSpace        ;         printf(" ");
            pop    eax              ;
            stosd                   ;         ARDStruct[j*4] = MemChkBuf[j*4];
            add    esi, 4           ;
            dec    edx              ;
            cmp    edx, 0           ;
            jnz    .1               ;     }
        call   DispReturn           ;     printf("\n");
        cmp    dword [dwType], 1    ;     if (Type == AddressRangeMemory)
        jne    .2                   ;     {
        mov    eax, [dwBaseAddrLow] ;
        add    eax, [dwLengthLow]   ;
        cmp    eax, [dwMemSize]     ;         if (BaseAddrLow + LengthLow > MemSize)
        jb     .2                   ;
        mov    [dwMemSize], eax     ;             MemSize = BaseAddrLow + LengthLow;
        .2:                         ;     }
            loop    .loop           ; }

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

SetupPaging:
    xor    edx, edx               ;
    mov    eax, [dwMemSize]       ;
    mov    ebx, 400000h           ; 400000h = 4M = 4096K * 1024, 一个页表对应的内存大小
    div    ebx                    ; PageTableNumber = dwMemSize / 4M
    mov    ecx, eax               ;
    test   edx, edx               ;
    jz     .no_remainder          ; if (dwMemSize % 4M)
    inc    ecx                    ;     ++PageTableNumber
                                  ;
    .no_remainder:                ;
    mov    [PageTableNumber], ecx ;
    mov    ax, SelectorFlatRW     ;
    mov    es, ax                 ;
    mov    edi, PageDirBase       ;
    xor    eax, eax               ;
    mov    eax, PageTblBase | PGN ;
    .1:                           ; for (i = PageTableNumber, addr = PageTblBase; i > 0; --i)
        stosd                     ;     [SelectorFlatRW:PageDirBase] = addr
        add    eax, 4096          ;     addr += 4K
        loop   .1                 ;     PageDirBase += 4 （双字 4 Byte）
    mov    eax, [PageTableNumber] ;
    mov    ebx, 1024              ;
    mul    ebx                    ;
    mov    ecx, eax               ; page_num = PageTableNumber * 1024
    mov    edi, PageTblBase       ;
    xor    eax, eax               ;
    mov    eax, PGN               ;
    .2:                           ; for (i = page_num, addr = 0; i > 0; --i)
        stosd                     ;     [SelectorFlatRW:PageTblBase] = addr
        add    eax, 4096          ;     addr += 4K
        loop   .2                 ;     PageTblBase += 4 （双字 4 Byte）

    mov    eax, PageDirBase
    mov    cr3, eax
    mov    eax, cr0
    or     eax, 80000000h
    mov    cr0, eax
    jmp    short .3
    .3:
    nop
    ret

InitKernel:
    xor    esi, esi                                 ;
    mov    cx, word [BaseOfKernelFilePhyAddr + 2ch] ; ┓ 44 bytes = 2ch
    movzx  ecx, cx                                  ; ┻ ecx = pELFHdr->e_phnum
    mov    esi, [BaseOfKernelFilePhyAddr + 1ch]     ; 28 bytes = 1ch
    add    esi, BaseOfKernelFilePhyAddr             ; esi = BaseOfKernelFilePhyAddr + pELFHdr->e_phoff
    .begin:                                         ; for (i = e_phnum; i > 0; --i) {
        mov    eax, [esi + 0]                       ;     \\ Elf32_Phdr->p_type
        cmp    eax, 0                               ;     if (p_type == 0)
        jz     .NoAction                            ;         continue;
        push   dword [esi + 010h]                   ;     \\ Elf32_Phdr->p_filesz
        mov    eax, [esi + 04h]                     ;     \\ Elf32_Phdr->p_offset
        add    eax, BaseOfKernelFilePhyAddr         ;
        push   eax                                  ;
        push   dword [esi + 08h]                    ;     \\ Elf32_Phdr->p_vaddr
        call   MemCpy                               ;     memcpy((void *)p_vaddr, B21r + p_offset, p_filesz);
        add    esp, 12                              ;
        .NoAction:                                  ;
            add    esi, 020h                        ;
            dec    ecx                              ;
            jnz    .begin                           ; }
    ret

%include  "lib_pm.inc"

[SECTION .data1]
ALIGN 32

LABEL_DATA:
    _szMemChkTitle:   db  "BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
    _szRAMSize:       db  "RAM size:", 0
    _szReturn:        db  0Ah, 0
    _szSpace:         db  " ", 0
    _dwMCRNumber:     dd  0
    _dwDispPos:       dd  (80 * 6 + 0) * 2
    _dwMemSize:       dd  0
    _PageTableNumber  dd  0
    _ARDStruct:
        _dwBaseAddrLow:   dd  0
        _dwBaseAddrHigh:  dd  0
        _dwLengthLow:     dd  0
        _dwLengthHigh:    dd  0
        _dwType:          dd  0
    _MemChkBuf: times   256 db  0

    szMemChkTitle    equ  BaseOfLoaderPhyAddr + _szMemChkTitle
    szRAMSize        equ  BaseOfLoaderPhyAddr + _szRAMSize
    szReturn         equ  BaseOfLoaderPhyAddr + _szReturn
    szSpace          equ  BaseOfLoaderPhyAddr + _szSpace
    dwDispPos        equ  BaseOfLoaderPhyAddr + _dwDispPos
    dwMemSize        equ  BaseOfLoaderPhyAddr + _dwMemSize
    dwMCRNumber      equ  BaseOfLoaderPhyAddr + _dwMCRNumber
    PageTableNumber  equ  BaseOfLoaderPhyAddr + _PageTableNumber
    ARDStruct        equ  BaseOfLoaderPhyAddr + _ARDStruct
        dwBaseAddrLow   equ  BaseOfLoaderPhyAddr + _dwBaseAddrLow
        dwBaseAddrHigh  equ  BaseOfLoaderPhyAddr + _dwBaseAddrHigh
        dwLengthLow     equ  BaseOfLoaderPhyAddr + _dwLengthLow
        dwLengthHigh    equ  BaseOfLoaderPhyAddr + _dwLengthHigh
        dwType          equ  BaseOfLoaderPhyAddr + _dwType
    MemChkBuf  equ  BaseOfLoaderPhyAddr + _MemChkBuf

    StackSpace:  times  1024  db  0
    TopOfStack   equ  BaseOfLoaderPhyAddr + $
