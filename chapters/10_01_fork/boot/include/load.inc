
BaseOfLoader             equ  09000h                 ; LOADER.BIN 被加载到的位置 - 段地址
OffsetOfLoader           equ   0100h                 ; LOADER.BIN 被加载到的位置 - 偏移地址
BaseOfLoaderPhyAddr      equ  BaseOfLoader * 10h     ; LOADER.BIN 被加载到的位置 - 物理地址

BaseOfKernelFile         equ  08000h                 ; KERNEL.BIN 被加载到的位置 - 段地址
OffsetOfKernelFile       equ      0h                 ; KERNEL.BIN 被加载到的位置 - 偏移地址
BaseOfKernelFilePhyAddr  equ  BaseOfKernelFile * 10h ; KERNEL.BIN 被加载到的位置 - 物理地址
KernelEntryPointPhyAddr  equ  030400h                ; KERNEL     被加载到的位置 - 物理地址

PageDirBase  equ  100000h ; 页目录开始地址: 1M
PageTblBase  equ  101000h ; 页表开始地址: 1M + 4K
