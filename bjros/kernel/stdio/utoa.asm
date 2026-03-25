; conversion of unsigned integer to char*
; author: Brian Juul Rasmussen aug 2020 
; arguments:
; 	unsigned integer to convert
;	buffer to contain chars
; version:
;	0.1
; Todo:
; 	some error checking
section .text
global _utoa:function
_utoa:
	push	ebp
	mov	ebp, esp	; store basepoiner on stack
	mov	ebx, 10		; divisor
	mov	ecx, 0		; counter
	mov	edx, ecx
	mov	eax, [ebp+8]	; arg1 - integer to convert to chars
	mov	edi, [ebp+12]	; arg2 - char buffer
	push	0		;
.countchars:	
	div	ebx		; divide edx:eax by 10 (ebx), result -> eax, remainder -> edx
	push	edx
	xor	edx, edx
	inc	ecx
	cmp	eax, 0		;
	je	.continue
	jmp	.countchars
.continue:
	mov	ebx, ebp
	sub	ebx, 4
	cmp	word [ebp-4], 2dh	;
	jne	.addchars		
	push	2dh-30h         
.addchars:
	pop	eax		;
	add	eax, 30h	;
	stosb	
	cmp	esp, ebx
	jne	.addchars
	mov	eax, 0		; store '\0' at end of string
	stosb			;		
	mov	eax, ecx	; return count of chars
	mov	esp, ebp
	pop	ebp

	ret
section .bss
section .data

	
