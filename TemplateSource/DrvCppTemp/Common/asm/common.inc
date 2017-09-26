
KFRAME_RPL  = -56h
KFRAME_CSR  = -54h
KFRAME_RAX  = -50h
KFRAME_RCX  = -48h
KFRAME_RDX  = -40h
KFRAME_R8   = -38h
KFRAME_R9   = -30h
KFRAME_R10  = -28h
KFRAME_R11  = -20h
KFRAME_XMM0 = -10h
KFRAME_XMM1 = +0h
KFRAME_XMM2 = +10h
KFRAME_XMM3 = +20h
KFRAME_XMM4 = +30h
KFRAME_XMM5 = +40h
KFRAME_SS   = +108h
KFRAME_RSP  = +100h
KFRAME_FLGS = +0F8h
KFRAME_CS   = +0F0h
KFRAME_EC   = +0E0h
KFRAME_IP   = +0E8h
KFRAME_RBX  = +0C0h
KFRAME_RDI  = +0C8h
KFRAME_RSI  = +0D0h
KFRAME_RBP  = +0D8h


; Saves all general purpose registers to the stack
; RSP is read from VMCS.
PUSHAQ MACRO
	push	r15
	push	r14
	push	r13
	push	r12
	push	r11
	push	r10
	push	r9
	push	r8
	push	rdi
	push	rsi
	push	rbp
	sub	rsp, 8	; placeholder
	push	rbx
	push	rdx
	push	rcx
	push	rax
ENDM

POPAQ MACRO
	pop	rax
	pop	rcx
	pop	rdx
	pop	rbx
	add	rsp, 8
	pop	rbp
	pop	rsi
	pop	rdi
	pop	r8
	pop	r9
	pop	r10
	pop	r11
	pop	r12
	pop	r13
	pop	r14
	pop	r15
ENDM

TRAP_SAVE_GPR MACRO
	; stack:
	;		ss (+40)
	;		rsp (+32)
	;		rflags (+24)
	;		cs (+16)
	;		ip (+8)	
	;		ec (+0)			<-- rsp

	push	rbp			; save rbp
	sub	rsp, 158h		; squeeze it to make shit fit
	lea	rbp, [rsp + 80h]

	; stack:
	;		ss	(+188h)
	;		rsp	(+180h)
	;		rflags  (+178h)
	;		cs	(+170h)
	;		ip	(+168h)
	;		ec	(+160h)
	;		rbp	(+158h)			<- original rbp saved
	;		frame	(+080h)			<- actual rbp pointing here
	;		data	(+000h)			<- rsp
	mov	[rbp + KFRAME_RAX], rax
	mov	[rbp + KFRAME_RCX], rcx 
	mov	[rbp + KFRAME_RDX], rdx
	mov	[rbp + KFRAME_R8], r8
	mov	[rbp + KFRAME_R9], r9
	mov	[rbp + KFRAME_R10], r10
	mov	[rbp + KFRAME_R11], r11
	mov [rbp + KFRAME_RBX], rbx
	mov [rbp + KFRAME_RDI], rdi
	mov [rbp + KFRAME_RSI], rsi
	mov rcx,cr2
	mov [rbp + 050h],rcx ;cr2也就是KFRAME的FaultAddress
ENDM


TRAP_SAVE_GPR_EX MACRO
	; stack:
	;		ss (+40)
	;		rsp (+32)
	;		rflags (+24)
	;		cs (+16)
	;		ip (+8)	
	;		ec (+0)			<-- rsp

	push rsi			; save rbp
	mov rsi, rbp;这里rsi是context
	sub	rsp, 150h		; squeeze it to make shit fit
	lea	rbp, [rsp + 80h]

	; stack:
	;		ss	(+188h)
	;		rsp	(+180h)
	;		rflags  (+178h)
	;		cs	(+170h)
	;		ip	(+168h)
	;		ec	(+160h)
	;		rbp	(+158h)			<- original rbp saved
	;		frame	(+080h)			<- actual rbp pointing here
	;		data	(+000h)			<- rsp
	mov	[rbp + KFRAME_RAX], rax
	mov	[rbp + KFRAME_RCX], rcx 
	mov	[rbp + KFRAME_RDX], rdx
	mov	[rbp + KFRAME_R8], r8
	mov	[rbp + KFRAME_R9], r9
	mov	[rbp + KFRAME_R10], r10
	mov	[rbp + KFRAME_R11], r11
	mov [rbp + KFRAME_RBX], rbx
	mov [rbp + KFRAME_RDI], rdi
	mov rcx,cr2
	mov [rbp + 050h],rcx ;cr2也就是KFRAME的FaultAddress
ENDM
; cleans up stack from TRAP_SAVE_GP.
TRAP_REST_GPR MACRO
	mov rsi,[rbp+KFRAME_RSI]
	mov rdi,[rbp+KFRAME_RDI]
	mov rbx,[rbp+KFRAME_RBX]
	mov	r11, [rbp + KFRAME_R11]
	mov	r10, [rbp + KFRAME_R10]
	mov	r9,  [rbp + KFRAME_R9]
	mov	r8,  [rbp + KFRAME_R8]
	mov	rdx, [rbp + KFRAME_RDX]
	mov	rcx, [rbp + KFRAME_RCX] 
	mov	rax, [rbp + KFRAME_RAX]

	mov	rsp, rbp
	mov	rbp, qword ptr [rbp + 0D8h]
	add	rsp, 0E8h
ENDM

TRAP_REST_GPR_EX MACRO
	mov rsi,[rbp+KFRAME_RSI]
	mov rdi,[rbp+KFRAME_RDI]
	mov rbx,[rbp+KFRAME_RBX]
	mov	r11, [rbp + KFRAME_R11]
	mov	r10, [rbp + KFRAME_R10]
	mov	r9,  [rbp + KFRAME_R9]
	mov	r8,  [rbp + KFRAME_R8]
	mov	rdx, [rbp + KFRAME_RDX]
	mov	rcx, [rbp + KFRAME_RCX] 
	mov	rax, [rbp + KFRAME_RAX]

	mov	rsp, rbp
	mov	rbp, qword ptr [rbp + 0D8h]
	add	rsp, 0E0h
ENDM
; save XMM registers and CSR
TRAP_SAVE_XMM MACRO
	stmxcsr	dword ptr [rbp + KFRAME_CSR]
	ldmxcsr	dword ptr gs:[180h]
	movaps	[rbp + KFRAME_XMM0], xmm0
	movaps	[rbp + KFRAME_XMM1], xmm1
	movaps	[rbp + KFRAME_XMM2], xmm2
	movaps	[rbp + KFRAME_XMM3], xmm3
	movaps	[rbp + KFRAME_XMM4], xmm4
	movaps	[rbp + KFRAME_XMM5], xmm5
ENDM

; cleans up XMM registers and CSR
TRAP_REST_XMM MACRO
	ldmxcsr	dword ptr[rbp + KFRAME_CSR]
	movaps	xmm0, xmmword ptr[rbp + KFRAME_XMM0]
	movaps	xmm1, xmmword ptr[rbp + KFRAME_XMM1]
	movaps	xmm2, xmmword ptr[rbp + KFRAME_XMM2]
	movaps	xmm3, xmmword ptr[rbp + KFRAME_XMM3]
	movaps	xmm4, xmmword ptr[rbp + KFRAME_XMM4]
	movaps	xmm5, xmmword ptr[rbp + KFRAME_XMM5]
ENDM

; General IDT trap handler (entry)
;	assumes:
;		1) There is an error code on the stack
;		2) NO_SWAP_LABEL is provided in case the trap is a kernel mode trap.
; Note: This does not save XMM registers, you need to do that with TRAP_SAVE_GP_XMM.
;
; Saves non-volatile registers on the frame pointer and jumps to NO_SWAP_LABEL if no
; GS swapping required (MSR_IA32_KERNEL_GS_BASE <-> MSR_IA32_GS_BASE), otherwise does
; swapgs and that's it.
;
; See __ept_violation below on how this is used.
TRAP_ENTER MACRO	NO_SWAP_LABEL, NO_ERROR_CODE
	IFNB <NO_ERROR_CODE>
		sub	rsp, 8
	ENDIF

	; align stack then save general purpose registers.
	TRAP_SAVE_GPR

	; see if we're coming from usermode, if so, swap gs.
	mov	ax, word ptr [rbp + KFRAME_CS]
	and	al, 1
	mov	[rbp + KFRAME_RPL], al
	jz	&NO_SWAP_LABEL&

	; ok we're coming from usermode, swap to kernel gs.
	swapgs

&NO_SWAP_LABEL&:
	; clear direction flag
	cld

	; save XMM
	TRAP_SAVE_XMM
ENDM

TRAP_ENTER_EX MACRO	NO_SWAP_LABEL

	TRAP_SAVE_GPR_EX

	; see if we're coming from usermode, if so, swap gs.
	mov	ax, word ptr [rbp + KFRAME_CS]
	and	al, 1
	mov	[rbp + KFRAME_RPL], al
	jz	&NO_SWAP_LABEL&

	; ok we're coming from usermode, swap to kernel gs.
	swapgs

&NO_SWAP_LABEL&:
	; clear direction flag
	cld

	; save XMM
	TRAP_SAVE_XMM
ENDM

TRAP_EXIT MACRO		NO_SWAP_LABEL
	; see if we're coming from usermode, if so, swap back gs
	test	byte ptr [rbp + KFRAME_RPL], 1
	jz	&NO_SWAP_LABEL&

	; ok we're coming from usermode
	TRAP_REST_XMM
	TRAP_REST_GPR
	swapgs
	iretq

&NO_SWAP_LABEL&:
	TRAP_REST_XMM
	TRAP_REST_GPR
	iretq
ENDM

TRAP_EXIT_EX MACRO		NO_SWAP_LABEL
	; see if we're coming from usermode, if so, swap back gs
	test	byte ptr [rbp + KFRAME_RPL], 1
	jz	&NO_SWAP_LABEL&

	; ok we're coming from usermode
	TRAP_REST_XMM
	TRAP_REST_GPR_EX
	swapgs
	iretq

&NO_SWAP_LABEL&:
	TRAP_REST_XMM
	TRAP_REST_GPR_EX
	iretq
ENDM

TRAP_EXIT_JMP MACRO	NO_SWAP_LABEL,JMP_C
	; see if we're coming from usermode, if so, swap back gs
	test	byte ptr [rbp + KFRAME_RPL], 1
	jz	&NO_SWAP_LABEL&

	; ok we're coming from usermode
	TRAP_REST_XMM
	TRAP_REST_GPR
	swapgs
	jmp &JMP_C&

&NO_SWAP_LABEL&:
	TRAP_REST_XMM
	TRAP_REST_GPR
	jmp &JMP_C&
ENDM

TRAP_EXIT_JMP_EX MACRO	NO_SWAP_LABEL,JMP_C
	; see if we're coming from usermode, if so, swap back gs
	test	byte ptr [rbp + KFRAME_RPL], 1
	jz	&NO_SWAP_LABEL&

	; ok we're coming from usermode
	TRAP_REST_XMM
	TRAP_REST_GPR_EX
	swapgs
	jmp &JMP_C&

&NO_SWAP_LABEL&:
	TRAP_REST_XMM
	TRAP_REST_GPR_EX
	jmp &JMP_C&
ENDM
NOP_32 MACRO
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop     ; 10
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop     ; 20
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop     ; 30
    nop
    nop
ENDM
