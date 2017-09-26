include ..\common\asm\common.inc
.CODE

InterruptRoutine PROC
jmp IntRoutineStart
vector_num  dq 0h
old_handler dq 0h
dispatcher dq 0h
IntRoutineStart:
TRAP_ENTER trap_no_swap,1
lea rsi,[vector_num]
lea rcx,[rbp-080h]
mov rdx,rsi
sub rsp,40h
call qword ptr [rsi+010h]
add rsp,40h
test eax,eax
jz jmpold
TRAP_EXIT trap_ret_no_swap
jmpold:
TRAP_EXIT_JMP trap_ret_no_swap1,old_handler
InterruptRoutine ENDP


InterruptRoutinePF PROC
jmp IntRoutineStart
vector_num_pf  dq 0h
old_pfhandler dq 0h
dispatcher_pf dq 0h
IntRoutineStart:
TRAP_ENTER trap_no_swap1
lea rsi,[vector_num_pf]
lea rcx,[rbp-080h]
mov rdx,rsi
sub rsp,40h
call qword ptr[rsi+010h]
add rsp,40h
test eax,eax
jz jmpold
TRAP_EXIT trap_ret_no_swap1
jmpold:
TRAP_EXIT_JMP_EX trap_ret_no_swap2,old_pfhandler
InterruptRoutinePF ENDP

END