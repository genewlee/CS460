        OSSEG  = 0x1000

       .globl _main,_running,_scheduler
       .globl _proc, _procSize
       .globl _tswitch,_resume

        jmpi   start,OSSEG

start:
    mov  ax,cs
    mov  ds,ax
    mov  ss,ax
    mov  es,ax
    mov  sp,#_proc
    add  sp,_procSize

    call _main

_tswitch:
SAVE:
    push ax
    push bx
    push cx
    push dx
    push bp
    push si
    push di
    pushf

    ! Saves CPU's SEGMENT registers
    push ds
    push ss

    mov  bx,_running ! bx -> proc
    mov  2[bx],sp    ! save sp to proc.ksp

FIND:   call _scheduler  ! call scheduler() in C

_resume:
RESUME:
    mov  bx,_running    ! bx -> running proc
    mov  sp,2[bx]       ! load sp with proc.ksp

    pop ss
    pop ds

    popf
    pop  di
    pop  si
    pop  bp
    pop  dx
    pop  cx
    pop  bx
    pop  ax

    ret
