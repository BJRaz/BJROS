; Name _strlen
; This program receives a string as argument 1, and return the length of the string
; Brian Juul Rasmussen jan 2020
;
; arguments:
;	char* - string to count length
; returns: 
;	strings length (without \0) in eax
; todo:
;	error handling 	
;
;
;
; tested sep 4. 2019 in gcc program.

section .text

global _strlen
_strlen:
	; read string address from stack
	; set esi to point to address
	; load char from esi, and compare to 0
	; if char != 0, increment length variable
	; if char == 0, set eax = length
	; return status to eax (>= 0 for ok)
	push dword ebp	; store old basepointer
	mov ebp, esp
	mov ecx,0x0

	mov esi, [ebp+8]
loop:
	lodsb		; read byte to al
	or al,al	; compare byte to 0
	jz end		; if zero, branch to end...
	
	inc ecx
	jmp loop	
end:		
	mov eax, ecx
	mov esp, ebp
	pop ebp	
	ret
section .data
	
	
