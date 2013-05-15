
global  memcpy
global  memset
global  strcpy
global  strlen

memcpy:
    push   ebp
    mov    ebp, esp
    push   esi
    push   edi
    push   ecx

    mov    edi, [ebp +  8] ; Destination
    mov    esi, [ebp + 12] ; Source
    mov    ecx, [ebp + 16] ; Counter
    .1:
        cmp    ecx, 0 ; 判断计数器
        jz     .2     ; 计数器为零时跳出

        mov    al, [ds:esi]      ; ┓
        inc    esi               ; ┃
                                 ; ┣ 逐字节移动
        mov    byte [es:edi], al ; ┃
        inc    edi               ; ┛

        dec    ecx ; 计数器减一
        jmp    .1  ; 循环
    .2:
        mov    eax, [ebp + 8] ; 返回值

    pop    ecx
    pop    edi
    pop    esi
    mov    esp, ebp
    pop    ebp

    ret; 函数结束，返回

memset:
    push   ebp
    mov    ebp, esp
    push   esi
    push   edi
    push   ecx

    mov    edi, [ebp + 8]  ; Destination
    mov    edx, [ebp + 12] ; Char
    mov    ecx, [ebp + 16] ; Counter
    .1:
        cmp    ecx, 0 ; 判断计数器
        jz     .2     ; 计数器为零时跳出

        mov    byte [edi], dl
        inc    edi

        dec    ecx ; 计数器减一
        jmp    .1  ; 循环
    .2:

    pop    ecx
    pop    edi
    pop    esi
    mov    esp, ebp
    pop    ebp

    ret; 函数结束，返回

strcpy:
    push   ebp
    mov    ebp, esp

    mov    esi, [ebp + 12]
    mov    edi, [ebp + 8]

    .1:
        mov    al, [esi]
        inc    esi
        mov    byte [edi], al
        inc    edi

        cmp    al, 0
        jnz    .1

    mov    eax, [ebp + 8]
    pop    ebp
    ret

strlen:
    push   ebp
    mov    ebp, esp

    mov    eax, 0
    mov    esi, [ebp + 8]

    .1:
        cmp    byte [esi], 0
        jz     .2
        inc    esi
        inc    eax
        jmp    .1
    .2:

    pop    ebp
    ret
