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
	mov	edx,ecx
	mov	eax, [ebp+8]	; arg1 - integer
.countchars:
	inc	ecx
	div	ebx		; divide edx:eax by 10 (ebx)
	jnz	.countchars
	
	mov	esp, ebp
	pop	ebp

	ret

