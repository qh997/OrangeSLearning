; 编译方法：nasm -o OSboot.bin OSboot.asm
; 制作镜像：dd if=OSboot.bin of=images/boot.img bs=512 count=1 conv=notrunc

;%define  _BOOT_DEBUG_

%ifdef  _BOOT_DEBUG_
    BaseOfStack  equ  0100h
%else
    BaseOfStack  equ  07c00h
%endif
SectorNoOfRootDir  equ  19     ; 根目录区的第一个扇区号
RootDirSectors     equ  14     ; 根目录区占用的扇区数（RootDirSectors' = (BPB_RootEntCnt * 32) / BPB_BytsPerSec）
BaseOfLoader       equ  09000h
OffsetOfLoader     equ  0100h
DeltaSectorNo      equ  17     ; 数据区开始的扇区号 - 根目录区占用的扇区数（RootDirSectors）
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

    mov    ax, 0600h  ; ┓
    mov    bx, 0700h  ; ┃
    mov    cx, 0      ; ┣ 清屏
    mov    dx, 0184fh ; ┃
    int    10h        ; ┛

    mov    dh, 0   ; "Booting  "
    call   DispStr

    xor    ah, ah ; ┓
    xor    dl, dl ; ┣ 软驱复位
    int    13h    ; ┛

    ; 在 A 盘的根目录寻找 LOADER.BIN
    mov    word [wSectorNo], SectorNoOfRootDir ; 从根目录的第一个扇区开始
    LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
        cmp    word [wRootDirSizeForLoop], 0 ; ┓ 根目录遍历结束
        jz     LABEL_NO_LOADERBIN            ; ┻ 没有找到 loader.bin
        dec    word [wRootDirSizeForLoop]

        mov    ax, BaseOfLoader   ; ┓
        mov    es, ax             ; ┃
        mov    bx, OffsetOfLoader ; ┃ es:bx = BaseOfLoader:OffsetOfLoader
        mov    ax, [wSectorNo]    ; ┃ 从第 19 个扇区开始
        mov    cl, 1              ; ┃ 一次只读一个扇区
        call   ReadSector         ; ┻ 将第 ax 个扇区开始的 cl 个扇区读入 es:bx 中

        mov    si, LoaderFileName ; ds:si -> "LOADER  BIN"
        mov    di, OffsetOfLoader ; es:di -> BaseOfLoader:OffsetOfLoader
        cld
        mov    dx, 10h ; 每个扇区一共有 512/32=16(10h) 个条目
        LABEL_SEARCH_FOR_LOADERBIN:
            cmp    dx, 0                              ; ┓ 该扇区遍历结束
            jz     LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR ; ┻ 没有找到 loader.bin
            dec    dx
            mov    cx, 11 ; "LOADER  BIN" 长度为 11 位
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
                    mov    si, LoaderFileName
                    jmp    LABEL_SEARCH_FOR_LOADERBIN

            LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
                add    word [wSectorNo], 1 ; 下一个扇区
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
        and    di, 0FFE0h ; ┓ 回到当前条目的起始位置
        add    di, 01Ah   ; ┻ 指向当前条目的起始簇号（DIR_FstClus）
        mov    cx, word [es:di] ; ┓
        push   cx               ; ┻ 保存开始的簇号（在 FAT 中的序号）
        add    cx, ax            ; ┓ cx = DeltaSectorNo + RootDirSectors + 文件的起始簇号
        add    cx, DeltaSectorNo ; ┻ cx = 文件的起始扇区号
        mov    ax, BaseOfLoader
        mov    es, ax
        mov    bx, OffsetOfLoader
        mov    ax, cx ; ax = 文件的起始扇区号

    LABEL_GOON_LOADING_FILE:
        push   ax
        push   bx
        mov    ah, 0Eh ; ┓
        mov    al, '.' ; ┃
        mov    bl, 0Fh ; ┣ 每读一个扇区就打印一个点 “.”
        int    10h     ; ┛

        pop    bx          ; ┓ es:bx = BaseOfLoader:OffsetOfLoader + n * BPB_BytsPerSec
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
            mov    dh, 1 ; "Ready.   "
            call   DispStr

    jmp    BaseOfLoader:OffsetOfLoader ; 跳转到内存中 loader.bin 开始处，并交出控制权

; 将第 ax 个扇区开始的 cl 个扇区读入 es:bx 中
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

; 找到序号为 ax 的 Sector 在 FAT 中的条目, 结果放在 ax 中
GetFATEntry:
    push   es
    push   bx
    push   ax
    mov    ax, BaseOfLoader ; ┓
    sub    ax, 0100h        ; ┣ 在 BaseOfLoader 后面留出 4K 空间用于存放 FAT
    mov    es, ax           ; ┛
    pop    ax
    mov    byte [bOdd], 0
    mov    bx, 3 ; ┓
    mul    bx    ; ┃ ax = ax * 3（每个 FAT 条目占 3 字节）
    mov    bx, 2 ; ┃
    div    bx    ; ┻ ax = 商, dx = 余数 
    cmp    dx, 0
    jz     LABEL_EVEN
    mov    byte [bOdd], 1

    LABEL_EVEN:
        xor    dx, dx               ; ┓ 现在 ax 中是 FATEntry 在 FAT 中的偏移量（字节）
        mov    bx, [BPB_BytsPerSec] ; ┃ ax / BPB_BytsPerSec
        div    bx                   ; ┣ ax = 商（FATEntry 所在的扇区相对于 FAT 的扇区号）
                                    ; ┛ dx = 余数（FATEntry 在扇区内的偏移）
        push   dx
        mov    bx, 0
        add    ax, SectorNoOfFAT1 ; ┓ ax = FATEntry 所在的扇区号
        mov    cl, 2              ; ┃ 一次读两个扇区
        call   ReadSector         ; ┻ 以防止 FATEntry 跨扇区
        pop    dx
        add    bx, dx
        mov    ax, [es:bx] ; 取出包含 FATEntry 的两个字节
        cmp    byte [bOdd], 1
        jnz    LABEL_EVEN_2
        shr    ax, 4     ; 偶数需要右移四位（BA987654 3210XXXX）
        LABEL_EVEN_2:
        and    ax, 0FFFh ; 奇数直接取（XXXXBA98 76543210）

    pop    bx
    pop    es
    ret

wSectorNo:            dw   0              ; 要读取的扇区号
wRootDirSizeForLoop:  dw   RootDirSectors ; 用于循环的根目录占用的扇区数, 在循环中会递减至零
bOdd:                 db   0

LoaderFileName:       db   "LOADER  BIN", 0
MessageLength         equ  9
BootMessage:          db   "Booting  "
Message1:             db   "Ready.   "
Message2:             db   "No LOADER"

times  510 - ($ - $$)  db  0
dw  0xaa55
