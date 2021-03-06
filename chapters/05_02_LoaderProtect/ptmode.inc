                        ; GD        P  S type
                        ; 0000 ---- 0000 0000
DA_32        EQU  4000h ; 0100
DA_4K        EQU  8000h ; 1000
DA_DPL0      EQU    00h ;           0000
DA_DPL1      EQU    20h ;           0010
DA_DPL2      EQU    40h ;           0100
DA_DPL3      EQU    60h ;           0110
DA_DR        EQU    90h ;           1001 0000
DA_DRW       EQU    92h ;           1001 0010
DA_DRWA      EQU    93h ;           1001 0011
DA_C         EQU    98h ;           1001 1000
DA_CR        EQU    9Ah ;           1001 1010
DA_CCO       EQU    9Ch ;           1001 1100
DA_CCOR      EQU    9Eh ;           1001 1110
DA_LDT       EQU    82h ;           1000 0010
DA_TaskGate  EQU    85h ;           1000 0101
DA_386TSS    EQU    89h ;           1000 1001
DA_386CGate  EQU    8Ch ;           1000 1100
DA_386IGate  EQU    8Eh ;           1000 1110
DA_386TGate  EQU    8Fh ;           1000 1111

                 ; -...- TI RPL
                 ; 15..3  2 10
SA_RPL0   EQU  0 ;          00
SA_RPL1   EQU  1 ;          01
SA_RPL2   EQU  2 ;          10
SA_RPL3   EQU  3 ;          11

PG_P      EQU  1 ; 页存在属性位
PG_RWR    EQU  0 ; R/W 属性位值, 读/执行
PG_RWW    EQU  2 ; R/W 属性位值, 读/写/执行
PG_USS    EQU  0 ; U/S 属性位值, 系统级
PG_USU    EQU  4 ; U/S 属性位值, 用户级

PGN       EQU  PG_P | PG_USU | PG_RWW

; 字符串打印启始位置
%define DispStart(row, col) (80 * row + col) * 2

; 描述符定义
%macro Descriptor 3
    dw  %2 & 0FFFFh
    dw  %1 & 0FFFFh
    db  (%1 >> 16) & 0FFh
    dw  ((%2 >> 8) & 0F00h) | (%3 & 0F0FFh)
    db  (%1 >> 24) & 0FFh
%endmacro
