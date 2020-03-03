; conversion of integer to char*
; arguments:
; 	integer to convert
;	buffer to contain chars
section .text
global _itoa
_itoa:
	push	ebp
	mov	ebp, esp	; store basepoiner on stack
	mov	ebx, 10		; divisor
	mov	ecx, 0		; counter
	mov	edx, ecx
	mov	eax, [ebp+8]	; arg1 - integer
	mov	edi, [ebp+12]	;
.countchars:	
	div	ebx		; divide edx:eax by 10 (ebx), result -> eax, remainder -> edx
	push	edx
	xor	edx, edx
	inc	ecx
	cmp	eax, 0		;
	je	.continue
	jmp	.countchars
.continue:
	pop	eax		;
	add	eax, 30h	;
	stosb	
	cmp	esp, ebp
	jne	.continue
	mov	eax, 0		; store '\0' at end of string
	stosb			;		
	mov	eax, ecx	; return count of chars
	mov	esp, ebp
	pop	ebp

	ret
section .bss
section .data

	
