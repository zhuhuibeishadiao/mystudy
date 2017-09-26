include common.inc
;EXTERN __handle_trap:PROC
;extern	old_trap_handler:qword
.CODE
;Trap PROC
;TRAP_ENTER trap_no_swap, 0
;
;	mov	rcx,	[rbp + KFRAME_CS]
;	mov	rdx,	[rbp + KFRAME_IP]
;	mov r8,		[rbp + KFRAME_EC]
;	mov r9,		cr2
;	sub	rsp,	40h
;	;call	__handle_trap
;	add	rsp,	40h
;	;rax=1 jmp 
;	test al,	al
;	jnz  jmp_old   
;TRAP_EXIT trap_ret_no_swap
;jmp_old:
;TRAP_EXIT_JMP trap_ret_no_swap1,old_trap_handler
;Trap ENDP
END