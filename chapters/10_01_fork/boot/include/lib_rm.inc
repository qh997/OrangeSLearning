
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

; 找到序号为 ax 的 Sector 在 FAT 中的条目, 结果放在 ax 中
GetFATEntry:
    push   es
    push   bx
    push   ax
    mov    ax, LOADER_SEG ; ┓
    sub    ax, 0100h      ; ┣ 在 BaseOfLoader 后面留出 4K 空间用于存放 FAT
    mov    es, ax         ; ┛
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

DispStrRM:
    mov    ax, MessageLength
    mul    dh
    add    ax, MessageStart
    mov    bp, ax
    mov    ax, ds
    mov    es, ax
    mov    cx, MessageLength
    mov    ax, 01301h
    mov    bx, 0007h
    add    dh, dl
    mov    dl, 0
    int    10h
    ret
