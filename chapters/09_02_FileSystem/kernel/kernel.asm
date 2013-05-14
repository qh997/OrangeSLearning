
%include "sconst.inc"

SELECTOR_KERNEL_CS  equ  8 ; LABEL_DESC_FLAT_C  - LABEL_GDT = 8

; 导入函数
    extern  cstart
    extern  exception_handler
    extern  kernel_main
    extern  disp_str

; 导入全局变量
    extern  disp_pos
    extern  gdt_ptr
    extern  idt_ptr
    extern  p_proc_ready
    extern  tss
    extern  k_reenter
    extern  irq_table
    extern  sys_call_table

BITS  32

[section .data]

[section .bss]
    StackSpace:  resb  2 * 1024 ; 2KB 的内核栈
    StackTop:

[section .text]
; 导出函数
    global  _start

    global  divide_error
    global  single_step_exception
    global  nmi
    global  breakpoint_exception
    global  overflow
    global  bounds_check
    global  inval_opcode
    global  copr_not_available
    global  double_fault
    global  copr_seg_overrun
    global  inval_tss
    global  segment_not_present
    global  stack_exception
    global  general_protection
    global  page_fault
    global  copr_error

    global  hwint00
    global  hwint01
    global  hwint02
    global  hwint03
    global  hwint04
    global  hwint05
    global  hwint06
    global  hwint07
    global  hwint08
    global  hwint09
    global  hwint10
    global  hwint11
    global  hwint12
    global  hwint13
    global  hwint14
    global  hwint15

    global  sys_call

    global  restart

; 内存示意图
    ;           ┃             ...            ┃
    ;           ┃                            ┃
    ;           ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
    ;           ┃■■■■■■■ Page  Tables ■■■■■■■┃
    ;           ┃■■■■ (Decide by LOADER) ■■■■┃ PageTblBase
    ; 00101000h ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
    ;           ┃■■■ Page Directory Table ■■■┃ PageDirBase = 1M
    ; 00100000h ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
    ;           ┃□□□□ Hardware  Reserved □□□□┃ B8000h ← gs
    ;    9FC00h ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
    ;           ┃■■■■■■■■ LOADER.BIN ■■■■■■■■┃ somewhere in LOADER ← esp
    ;    90000h ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
    ;           ┃■■■■■■■■ KERNEL.BIN ■■■■■■■■┃
    ;    80000h ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
    ;           ┃■■■■■■■■■■ KERNEL ■■■■■■■■■■┃ 30400h ← KERNEL 入口 (KernelEntryPointPhyAddr)
    ;    30000h ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
    ;           ┋            ...             ┋
    ;           ┋                            ┋
    ;        0h ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ ← cs, ds, es, fs, ss

_start:
    mov    esp, StackTop       ; 切换堆栈
    mov    dword [disp_pos], 0
    
    sgdt   [gdt_ptr] ; ┓
    call   cstart    ; ┣ 更换 GDT
    lgdt   [gdt_ptr] ; ┛

    lidt   [idt_ptr] ; 加载 IDT

    jmp    SELECTOR_KERNEL_CS:csinit ; 这个跳转指令强制使用刚刚初始化的结构

    csinit:
        xor    eax, eax         ; ┓
        mov    ax, SELECTOR_TSS ; ┣ 加载 TSS
        ltr    ax               ; ┛

        jmp    kernel_main

%macro  hwint_master 1
    call   save

    in     al, INT_M_CTLMASK ; ┓
    or     al, (1 << %1)     ; ┣ 屏蔽该中断
    out    INT_M_CTLMASK, al ; ┛

    mov    al, EOI           ; ┓
    out    INT_M_CTL, al     ; ┻ 继续接收中断

    sti                         ; 开中断
    push   %1                   ;
    call   [irq_table + 4 * %1] ; ((irq_handler *)irq_table[irq])(irq)
    pop    ecx                  ;
    cli                         ; 关中断

    in     al, INT_M_CTLMASK ; ┓
    and    al, ~(1 << %1)    ; ┣ 恢复该中断
    out    INT_M_CTLMASK, al ; ┛

    ret ; 此时将跳转至在 save 中 push 的地址（restart/restart_reenter）
%endmacro
%macro  hwint_slave 1
    call   save

    in     al, INT_S_CTLMASK   ; ┓
    or     al, (1 << (%1 - 8)) ; ┣ 屏蔽该中断
    out    INT_S_CTLMASK, al   ; ┛

    mov    al, EOI             ; ┓
    out    INT_M_CTL, al       ; ┃
    nop                        ; ┣ 继续接收中断
    out    INT_S_CTL, al       ; ┛

    sti
    push   %1
    call   [irq_table + 4 * %1] ; ((irq_handler *)irq_table[irq])(irq)
    pop    ecx
    cli

    in     al, INT_S_CTLMASK    ; ┓
    and    al, ~(1 << (%1 - 8)) ; ┣ 恢复该中断
    out    INT_S_CTLMASK, al    ; ┛

    ret ; 此时将跳转至在 save 中 push 的地址（restart/restart_reenter）
%endmacro
; 硬件中断
    ALIGN  16
    hwint00:            ; <clock>
        hwint_master  0
    ALIGN  16
    hwint01:            ; <keyboard>
        hwint_master  1
    ALIGN  16
    hwint02:            ; <cascade!>
        hwint_master  2
    ALIGN  16
    hwint03:            ; <second serial>
        hwint_master  3
    ALIGN  16
    hwint04:            ; <first serial>
        hwint_master  4
    ALIGN  16
    hwint05:            ; <XT winchester>
        hwint_master  5
    ALIGN  16
    hwint06:            ; <floppy>
        hwint_master  6
    ALIGN  16
    hwint07:            ; <printer>
        hwint_master  7
    ALIGN  16
    hwint08:            ; <realtime clock>
        hwint_slave   8
    ALIGN  16
    hwint09:            ; <irq 2 redirected>
        hwint_slave   9
    ALIGN  16
    hwint10:            ; 
        hwint_slave  10
    ALIGN  16
    hwint11:            ; 
        hwint_slave  11
    ALIGN  16
    hwint12:            ; 
        hwint_slave  12
    ALIGN  16
    hwint13:            ; <FPU exception>
        hwint_slave  13
    ALIGN  16
    hwint14:            ; <AT winchester>
        hwint_slave  14
    ALIGN  16
    hwint15:            ; 
        hwint_slave  15

; 异常
    divide_error:
        push   0xFFFFFFFF
        push   0
        jmp    exception
    single_step_exception:
        push   0xFFFFFFFF
        push   1
        jmp    exception
    nmi:
        push   0xFFFFFFFF
        push   2
        jmp    exception
    breakpoint_exception:
        push   0xFFFFFFFF
        push   3
        jmp    exception
    overflow:
        push   0xFFFFFFFF
        push   4
        jmp    exception
    bounds_check:
        push   0xFFFFFFFF
        push   5
        jmp    exception
    inval_opcode:
        push   0xFFFFFFFF
        push   6
        jmp    exception
    copr_not_available:
        push   0xFFFFFFFF
        push   7
        jmp    exception
    double_fault:
        push   8
        jmp    exception
    copr_seg_overrun:
        push   0xFFFFFFFF
        push   9
        jmp    exception
    inval_tss:
        push   10
        jmp    exception
    segment_not_present:
        push   11
        jmp    exception
    stack_exception:
        push   12
        jmp    exception
    general_protection:
        push   13
        jmp    exception
    page_fault:
        push   14
        jmp    exception
    copr_error:
        push   0xFFFFFFFF
        push   16
        jmp    exception

    exception:
        call   exception_handler
        add    esp, 4 * 2
        hlt

save:
    ; 此时 eip/cs/eflags/esp/ss 已经压栈
    ; call save 的返回地址也压栈，esp 指向 proc->regs.eax
    pushad        ; ┓
    push   ds     ; ┃
    push   es     ; ┣ 保护现场
    push   fs     ; ┃
    push   gs     ; ┛

    mov    esi, edx ; 暂存 edx（第四个参数）
    mov    dx, ss
    mov    ds, dx
    mov    es, dx
    mov    fs, dx
    mov    edx, esi

    mov    esi, esp                     ; 此时 esi 为进程表起始地址

    inc    dword [k_reenter]
    cmp    dword [k_reenter], 0         ; 如果 k_reenter != 0，则表示中断重入
    jne    .1                           ; k_reenter != 0
    mov    esp, StackTop                ; 切换到内核栈，在此之前绝对不能进行栈操作
    push   restart
    jmp    [esi + RETADR - P_STACKBASE] ; 返回
    .1:
        push   restart_reenter
    .2:
    jmp    [esi + RETADR - P_STACKBASE] ; 返回
                                        ; 由于存在栈切换并且压栈的值没有弹出
                                        ; 所以不能使用 ret 直接返回。

sys_call:
    call   save
    sti

    push   esi

    push   edx
    push   ecx
    push   ebx
    push   dword [p_proc_ready]
    call   [sys_call_table + eax * 4]
    add    esp, 4 * 4

    pop    esi
    mov    [esi + EAXREG - P_STACKBASE], eax ; 返回值，proc->regs.eax = eax

    cli
    ret ; 此时将跳转至在 save 中 push 的地址（restart/restart_reenter）

restart: ; 非中断重入
    mov    esp, [p_proc_ready]           ; 切换到进程表栈
    lldt   [esp + P_LDT_SEL]             ; 加载该进程的 LDT
    lea    eax, [esp + P_STACKTOP]       ; ┓ 下一次中断发生时，esp 将变成 s_proc.regs 的末地址
    mov    dword [tss + TSS3_S_SP0], eax ; ┻ 然后再将该进程的 ss/esp/eflags/cs/eip 压栈
restart_reenter: ; 中断重入
    dec    dword [k_reenter]
    pop    gs     ; ┓
    pop    fs     ; ┃
    pop    es     ; ┃
    pop    ds     ; ┣ 恢复现场
    popad         ; ┛
    add    esp, 4 ; 跳过 retaddr
    iretd         ; pop eip/cs/eflags/esp/ss
