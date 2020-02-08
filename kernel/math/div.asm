section .text
global _start
_start:	
	mov	edi, 10b	; 2
	mov	eax, 76 
	mov	ebx, 0		; counter
loop:	
	div	edi
	push 	edx
	mov	edx, 0
	inc	ebx
	cmp 	eax, 0
	je	end_loop
	jmp	loop
end_loop:

	mov	eax, 1	; linux exit syscall
	mov	ebx, 0
	int	0x80	; interrupt syscall



	
