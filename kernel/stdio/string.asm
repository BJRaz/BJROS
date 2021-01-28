; string functions:
; _strcmp(str1, str2) returns int (0 if equal)
; 
section .text

global	_strcmp:function
_strcmp:
	push	ebp
	mov	ebp, esp
	
	;pushad
	push	ebx
	push	edi
	push	esi

	; str1 : ebp+8
	; str2 : ebp+12
	mov	esi, [ebp+8]
	mov	edi, [ebp+12]
	mov	ebx, 0
.read:
	lodsb	; store byte at esi in AL
	cmp 	al, [edi]
	je	.match

	mov	ebx, 1
	jmp	.out
.match
	cmp	al, 0
	je	.out

	; OK
	inc	edi	
	jmp	.read
.out:
	;cmp	[edi], 0
	;je	.finish		; strings matches in length
	mov	eax, ebx
	pop	esi
	pop	edi
	pop	ebx
	;popad
	leave
	ret
	
