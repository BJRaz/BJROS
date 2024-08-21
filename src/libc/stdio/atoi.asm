; implementation of atoi
; author: Brian Juul Rasmussen jan 2020
; version:
;	0.1
;	0.2 added check for negative numbers
; converts an (signed) ascii number i.e. 842 to base 10
; 
; arguments:
; 	char*  - ascii string to convert.
; returns:
;	the converted number as base 10 in eax
;
; todo:
;	implement conversion of signed numbers	
;	error handling
;
section .text
global _atoi:function
_atoi:
	; ascii char string as only argument  
	
	push	ebp		; store basepointer
	mov	ebp, esp	; set basepointer = stack pointer
	mov	esi, [ebp+8]	; set esi to point to char* on stack
	mov 	edi, esi	;
	push	0		;
	xor	eax, eax
	mov	ebx, eax	;
	cmp	esi, 0		; check if argument points to 0 (empty string)
	je	.cont
	cmp 	byte [esi], 2Dh	; ascii 2Dh eq '-' (minus)
	jne	.init		;
.negative:
	inc 	esi		; increment esi pass '-'
	mov 	edi, esi	;
	mov	word [esp], 1	; bool value indicates negative number		
.init:
	mov	ecx, 1		; mulplicator
.loop:
	cmp	byte [esi], 0	; if end of string '\0'
	je	.calc
	;push 	ax		; 
	inc	esi		; increments esi
	jmp	.loop
.calc:
	cmp	edi,esi		; reached end of string ?
	je	.cont	

	dec	esi		;	
	xor	eax, eax	; set eax = 0

	mov	byte al, [esi]	; 
	sub	eax, 30h	; convert to ascii code
	mul	ecx		; 
	add	ebx, eax	; ebx += eax
	mov	eax, ecx	; moves ecx til eax
	mov	ecx, 10		; 
	mul	ecx		; eax = ecx * eax
	mov	ecx, eax	; set multiplicator 
	jmp	.calc
.cont:
	cmp	word [esp], 1
	jne	.leave
	not	ebx		; 2 complement calculation
	add	ebx, 1		;		
.leave:
	mov	eax, ebx	; returns ebx ascii -> int	
	mov	esp, ebp	; restore basepointer
	pop	ebp

	ret
	
section .data
section .bss

