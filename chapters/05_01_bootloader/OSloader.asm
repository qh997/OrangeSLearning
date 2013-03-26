org    0100h
jmp    LABEL_START

BaseOfStack  equ  0100h
%include  "fat12.inc"

LABEL_START:
    mov    ax, cs
    mov    ds, ax
    mov    es, ax
    mov    ss, ax
    mov    sp, BaseOfStack

    mov    dx, 0003h
    call   DispStrRM

    xor    ah, ah ; ┓
    xor    dl, dl ; ┣ 软驱复位
    int    13h    ; ┛

    mov    word [wSectorNo], SectorNoOfRootDir
    LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
        cmp    word [wRootDirSizeForLoop], 0 ; ┓ 根目录遍历结束
        jz     LABEL_NO_KERNELBIN            ; ┻ 没有找到 loader.bin
        dec    word [wRootDirSizeForLoop]

        mov    ax, BaseOfKernel   ; ┓
        mov    es, ax             ; ┃
        mov    bx, OffsetOfKernel ; ┃ es:bx = BaseOfKernel:OffsetOfKernel
        mov    ax, [wSectorNo]    ; ┃ 从第 19 个扇区开始
        mov    cl, 1              ; ┃ 一次只读一个扇区
        call   ReadSector         ; ┻ 将第 ax 个扇区开始的 cl 个扇区读入 es:bx 中

        mov    si, KernelFileName     ; ds:si -> "KERNEL  BIN"
        mov    di, OffsetOfKernel ; es:di -> BaseOfKernel:OffsetOfKernel
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
        mov    ax, BaseOfKernel
        mov    es, ax
        mov    bx, OffsetOfKernel
        mov    ax, cx ; ax = 文件的起始扇区号

    LABEL_GOON_LOADING_FILE:
        push   ax
        push   bx
        mov    ah, 0Eh ; ┓
        mov    al, '.' ; ┃
        mov    bl, 0Fh ; ┣ 每读一个扇区就打印一个点 “.”
        int    10h     ; ┛

        pop    bx          ; ┓ es:bx = BaseOfKernel:OffsetOfKernel + n * BPB_BytsPerSec
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

            jmp    $

%include  "lib.inc"

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
