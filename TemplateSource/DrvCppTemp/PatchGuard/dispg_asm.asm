include ..\common\asm\common.inc
EXTERN handle_pagefault_trap:PROC
EXTERN handle_debugfault_trap:PROC
extern old_pftrap_handler:qword
extern old_debugfault_handler:qword
.CODE
pagefault_trap PROC
TRAP_ENTER trap_no_swap
lea rcx,[rbp-080h]
sub rsp,40h
call handle_pagefault_trap
add rsp,40h
test eax,eax
jz jmpold
TRAP_EXIT trap_ret_no_swap
jmpold:
TRAP_EXIT_JMP_EX trap_ret_no_swap1,old_pftrap_handler
pagefault_trap ENDP
debugfault_trap PROC
TRAP_ENTER trap_no_swap,0
lea rcx,[rbp-080h]
sub rsp,40h
call handle_debugfault_trap
add rsp,40h
test eax,eax
jz jmpold
TRAP_EXIT trap_ret_no_swap
jmpold:
TRAP_EXIT_JMP trap_ret_no_swap1,old_debugfault_handler
debugfault_trap ENDP

PF_Test PROC
mov rax,0ABCDABCDh
mov [rax],rax
ret
PF_Test ENDP
END