
DispReturn:
    push   eax
    push   ebx
    mov    eax, edi
    mov    bl, 160
    div    bl
    and    eax, 0FFh
    inc    eax
    mov    bl, 160
    mul    bl
    mov    edi, eax
    pop    ebx
    pop    eax
    ret

DispAL:
    push   ecx
    push   edx
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
    add    edi, 2
    pop    edx
    pop    ecx
    ret