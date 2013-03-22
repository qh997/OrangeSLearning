; 编译方法：nasm -o OSboot.bin OSboot.asm
; 制作镜像：dd if=OSboot.bin of=images/boot.img bs=512 count=1 conv=notrunc

;%define  _BOOT_DEBUG_

%ifdef  _BOOT_DEBUG_
    BaseOfStack  equ  0100h
%else
    BaseOfStack  equ  07c00h
%endif
SectorNoOfRootDir  equ  19
RootDirSectors     equ  14
BaseOfLoader       equ  09000h
OffsetOfLoader     equ  0100h
DeltaSectorNo      equ  17
SectorNoOfFAT1     equ  1

%ifdef  _BOOT_DEBUG_
    org    0100h
%else
    org    07c00h
%endif

jmp    short  LABEL_START
nop

; FAT12
    BS_OEMName      DB  'ForrestY'     ; OEM String, 必须 8 个字节
    BPB_BytsPerSec  DW  512            ; 每扇区字节数
    BPB_SecPerClus  DB  1              ; 每簇多少扇区
    BPB_RsvdSecCnt  DW  1              ; Boot 记录占用多少扇区
    BPB_NumFATs     DB  2              ; 共有多少 FAT 表
    BPB_RootEntCnt  DW  224            ; 根目录文件数最大值
    BPB_TotSec16    DW  2880           ; 逻辑扇区总数
    BPB_Media       DB  0xF0           ; 媒体描述符
    BPB_FATSz16     DW  9              ; 每 FAT 扇区数
    BPB_SecPerTrk   DW  18             ; 每磁道扇区数
    BPB_NumHeads    DW  2              ; 磁头数(面数)
    BPB_HiddSec     DD  0              ; 隐藏扇区数
    BPB_TotSec32    DD  0              ; wTotalSectorCount 为 0 时这个值记录扇区数
    BS_DrvNum       DB  0              ; 中断 13 的驱动器号
    BS_Reserved1    DB  0              ; 未使用
    BS_BootSig      DB  29h            ; 扩展引导标记 (29h)
    BS_VolID        DD  0              ; 卷序列号
    BS_VolLab       DB  'OrangeS0.02'  ; 卷标, 必须 11 个字节
    BS_FileSysType  DB  'FAT12   '     ; 文件系统类型, 必须 8个字节  

LABEL_START:
    mov    ax, cs
    mov    ds, ax
    mov    es, ax
    mov    ss, ax
    mov    sp, BaseOfStack

    mov    ax, 0600h
    mov    bx, 0700h
    mov    cx, 0
    mov    dx, 0184fh
    int    10h
    mov    dh, 0
    call   DispStr

    xor    ah, ah
    xor    dl, dl
    int    13h

    mov    word [wSectorNo], SectorNoOfRootDir
    LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
        cmp    word [wRootDirSizeForLoop], 0
        jz     LABEL_NO_LOADERBIN
        dec    word [wRootDirSizeForLoop]
        mov    ax, BaseOfLoader
        mov    es, ax
        mov    bx, OffsetOfLoader
        mov    ax, [wSectorNo]
        mov    cl, 1
        call   ReadSector

        mov    si, LoaderFileName
        mov    di, OffsetOfLoader
        cld
        mov    dx, 10h
        LABEL_SEARCH_FOR_LOADERBIN:
            cmp    dx, 0
            jz     LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
            dec    dx
            mov    cx, 11
            LABEL_CMP_FILENAME:
                cmp    cx, 0
                jz     LABEL_FILENAME_FOUND
                dec    cx
                lodsb
                cmp    al, byte [es:di]
                jz     LABEL_GO_ON
                jmp    LABEL_DIFFERENT

                LABEL_GO_ON:
                    inc    di
                    jmp    LABEL_CMP_FILENAME

                LABEL_DIFFERENT:
                    and    di, 0FFE0h
                    add    di, 20h
                    mov    si, LoaderFileName
                    jmp    LABEL_SEARCH_FOR_LOADERBIN

            LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
                add    word [wSectorNo], 1
                jmp    LABEL_SEARCH_IN_ROOT_DIR_BEGIN

    LABEL_NO_LOADERBIN:
        mov    dh, 2
        call   DispStr

        %ifdef  _BOOT_DEBUG_
            mov    ax, 4c00h
            int    21h
        %else
            jmp    $
        %endif

    LABEL_FILENAME_FOUND:
        mov    ax, RootDirSectors
        and    di, 0FFE0h
        add    di, 01Ah
        mov    cx, word [es:di]
        push   cx
        add    cx, ax
        add    cx, DeltaSectorNo
        mov    ax, BaseOfLoader
        mov    es, ax
        mov    bx, OffsetOfLoader
        mov    ax, cx

    LABEL_GOON_LOADING_FILE:
        push   ax
        push   bx
        mov    ah, 0Eh
        mov    al, '.'
        mov    bl, 0Fh
        int    10h
        pop    bx
        pop    ax

        mov    cl, 1
        call   ReadSector
        pop    ax
        call   GetFATEntry
        cmp    ax, 0FFFh
        jz     LABEL_FILE_LOADED
        push   ax
        mov    dx, RootDirSectors
        add    ax, dx
        add    ax, DeltaSectorNo
        add    bx, [BPB_BytsPerSec]
        jmp    LABEL_GOON_LOADING_FILE

        LABEL_FILE_LOADED:
            mov    dh, 1
            call   DispStr

    jmp    BaseOfLoader:OffsetOfLoader

ReadSector:
    push   bp
    mov    bp, sp
    sub    esp, 2

    mov    byte [bp - 2], cl
    push   bx
    mov    bl, [BPB_SecPerTrk]
    div    bl
    inc    ah
    mov    cl, ah
    mov    dh, al
    shr    al, 1
    mov    ch, al
    and    dh, 1

    pop    bx
    mov    dl, [BS_DrvNum]
    .GoOnReading:
        mov    ah, 2
        mov    al, byte [bp - 2]
        int    13h
        jc     .GoOnReading
    add    esp, 2
    pop    bp

    ret

DispStr:
    mov    ax, MessageLength
    mul    dh
    add    ax, BootMessage
    mov    bp, ax
    mov    ax, ds
    mov    es, ax
    mov    cx, MessageLength
    mov    ax, 01301h
    mov    bx, 0007h
    mov    dl, 0
    int    10h
    ret

GetFATEntry:
    push   es
    push   bx
    push   ax
    mov    ax, BaseOfLoader
    sub    ax, 0100h
    mov    es, ax
    pop    ax
    mov    byte [bOdd], 0
    mov    bx, 3
    mul    bx
    mov    bx, 2
    div    bx
    cmp    dx, 0
    jz     LABEL_EVEN
    mov    byte [bOdd], 1

    LABEL_EVEN:
        xor    dx, dx
        mov    bx, [BPB_BytsPerSec]
        div    bx
        push   dx
        mov    bx, 0
        add    ax, SectorNoOfFAT1
        mov    cl, 2
        call   ReadSector
        pop    dx
        add    bx, dx
        mov    ax, [es:bx]
        cmp    byte [bOdd], 1
        jnz    LABEL_EVEN_2
        shr    ax, 4
        LABEL_EVEN_2:
            and    ax, 0FFFh

    pop    bx
    pop    es
    ret

wSectorNo:            dw   0
wRootDirSizeForLoop:  dw   RootDirSectors
bOdd:                 db   0

LoaderFileName:       db   "LOADER  BIN", 0
MessageLength         equ  9
BootMessage:          db   "Booting  "
Message1:             db   "Ready.   "
Message2:             db   "No LOADER"

times  510 - ($ - $$)  db  0
dw  0xaa55
