
BaseOfStack  equ  07c00h

org    07c00h
jmp    short LABEL_START
nop

%include  "fat12.inc"
%include  "load.inc"

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

    mov    dx, 0000h  ; "Booting  "
    call   DispStrRM

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
        mov    dx, 0200h
        call   DispStrRM
        jmp    $

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
            mov    dx, 0100h ; "Ready.   "
            call   DispStrRM

    jmp    BaseOfLoader:OffsetOfLoader ; 跳转到内存中 loader.bin 开始处，并交出控制权

%include  "lib_rm.inc"

wSectorNo:            dw   0              ; 要读取的扇区号
wRootDirSizeForLoop:  dw   RootDirSectors ; 用于循环的根目录占用的扇区数, 在循环中会递减至零
bOdd:                 db   0

LoaderFileName:       db   "LOADER  BIN", 0
MessageLength         equ  9
MessageStart:         db   "Booting  "
Message1:             db   "Ready.   "
Message2:             db   "No LOADER"

times  510 - ($ - $$)  db  0
dw  0xaa55
