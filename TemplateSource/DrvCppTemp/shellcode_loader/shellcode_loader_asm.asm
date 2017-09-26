include ..\common\asm\common.inc
.CODE
shellcode_retn PROC
mov rax,rdx
ret
shellcode_retn ENDP

END