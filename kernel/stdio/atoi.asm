; implementation of atoi
section .text
global _atoi
_atoi:
	; ascii char string as only argument  
	
	push	ebp		; store basepointer
	mov	ebp, esp	; set basepointer = stack pointer
	mov	esi, [ebp+8]	; set esi to point to char* on stack
	mov 	edi, esi	;
	xor	eax, eax
	mov	ebx, eax	;
	mov	ecx, 1
.loop:
	cmp	byte [esi], 0
	je	.calc
	;push 	ax	;
	inc	esi
	jmp	.loop
.calc:
	cmp	edi,esi
	je	.cont	

	dec	esi		;	
	xor	eax, eax	; check

	mov	byte al, [esi]	
	sub	eax, 30h	; convert to ascii code
	mul	ecx
	add	ebx, eax	; ebx += eax
	mov	eax,ecx		; moves ecx til eax
	mov	ecx, 10		; 
	mul	ecx		; eax = ecx * eax
	mov	ecx, eax	;
	jmp	.calc
.cont:
	
		
	mov	eax, ebx	; returns ebx ascii -> int	

	mov	esp, ebp	; restore basepointer
	pop	ebp

	ret
	
section .data
section .bss

