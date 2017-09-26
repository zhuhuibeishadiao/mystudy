include ..\Common\asm\common.inc
EXTERN HandleBreakPoint:PROC
extern old_breakpoint_handler:qword
.CODE
breakpoint_trap PROC
TRAP_ENTER trap_no_swap2,1
sti
lea rcx,[rbp-080h]
sub rsp,40h
call HandleBreakPoint
add rsp,40h
test eax,eax
jz jmpold
TRAP_EXIT trap_ret_no_swap
jmpold:
TRAP_EXIT_JMP trap_ret_no_swap1,old_breakpoint_handler
breakpoint_trap ENDP
END