DispStr:
    push   ebp
    mov    ebp, esp
    push   eax
    push   ebx
    push   esi
    push   edi

    mov    esi, [ebp + 8]
    mov    edi, [dwDispPos]
    mov    ah, 0CH
    .1:
        lodsb
        test   al, al
        jz     .2
        cmp    al, 0Ah
        jnz    .3
        push   eax
        mov    eax, edi
        mov    bl, 160
        div    bl
        and    eax, 0FFh
        inc    eax
        mov    bl, 160
        mul    bl
        mov    edi, eax
        pop    eax
        jmp    .1
    .3:
        mov    [gs:edi], ax
        add    edi, 2
        jmp    .1
    .2:
        mov    [dwDispPos], edi

    pop    edi
    pop    esi
    pop    ebx
    pop    eax
    pop    ebp

    ret

DispReturn:
    push   szReturn
    call   DispStr
    add    esp, 4
    ret

DispSpace:
    push   szSpace
    call   DispStr
    add    esp, 4
    ret

DispAL:
    push   ecx
    push   edx
    push   edi

    mov    edi, [dwDispPos]

    mov    ah, 0Ch
    mov    dl, al
    shr    al, 4
    mov    ecx, 2
    .begin:
        and    al, 0Fh
        cmp    al, 9
        ja     .1
        add    al, '0'
        jmp    .2
    .1:
        sub    al, 0Ah
        add    al, 'A'
    .2:
        mov    [gs:edi], ax
        add    edi, 2

        mov    al, dl
        loop   .begin
    mov    [dwDispPos], edi

    pop    edi
    pop    edx
    pop    ecx

    ret

DispInt:
    mov    eax, [esp + 4]
    shr    eax, 24
    call   DispAL

    mov    eax, [esp + 4]
    shr    eax, 16
    call   DispAL

    mov    eax, [esp + 4]
    shr    eax, 8
    call   DispAL

    mov    eax, [esp + 4]
    call   DispAL

    mov    ah, 07h
    mov    al, 'h'
    push   edi
    mov    edi, [dwDispPos]
    mov    [gs:edi], ax
    add    edi, 2
    mov    [dwDispPos], edi
    pop    edi

    ret
