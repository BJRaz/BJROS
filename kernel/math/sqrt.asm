; dette er kun test misc tests af NASM

; extern angives hvis funktionen er implementeret i et andet (shared) library - ellers vil assembleren fejle
extern	printf
extern 	sqrt

section .text
global _start
_start:
	push dword 0
	push dword 0

	mov edi,storage

	push dword 0 		; put 0 on stack for sqrt
	push dword 144		; put 144 on stack for sqrt
	call sqrt		; calls sqrt (squareroot) in libm (libmath)
	add esp, byte 4

	push dword eax
	push dword format
	call printf
	add esp, byte 4

	mov ebx, eax		; value to exit
	mov eax, 1		; linux exit system call
	int 0x80		; call syscall
section .data
	format db "Returned number is %f",10,0
section .bss
	storage	resw	4
	
