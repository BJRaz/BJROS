;org 0x100000				; addresses start origin - only enable if format is binary	
					; implemented by BJR dec 2019/ jan 2020
bits 32					; forces nasm to generate a 32-bit image for 32-bit processor protected mode

extern kmain				; kmail is kernel main function ing if binary format.

section .text
global _start
_start:
	jmp	multiboot_entry
	align 4				; 32 bit aligned	
multiboot_header:
	dd	MB_HEADER_MAGIC		; magic number
	dd	MB_HEADER_FLAGS		; flags
	dd	MB_HEADER_CHECKSUM
	dd	multiboot_header	; header address
	dd	_start			; load start of .text secment
	dd	0			; load end addr.
	dd	0			; bss end addr.
	dd	multiboot_entry		; entry addr.
multiboot_entry:
	push	eax			; contains magic value (magic number)
	push	ebx			; address of multiboot structure
	
 	call 	kmain			;_kmain			; call kernel main function

	mov	eax, cs			; move content of Code Section to eax (cs = hidden, visible)

	hlt
_kmain:	
	; do some work (test only)
	mov 	dword [0xb8000],videotext ; writes text on screen	
	mov	eax, 16			; dummy kernel main function
	ret

section .data
MB_HEADER_MAGIC		equ	0x1badb002
MB_HEADER_FLAGS		equ	0x00010003
MB_HEADER_CHECKSUM	equ	-(MB_HEADER_MAGIC+MB_HEADER_FLAGS)
videotext		equ	0xff65ff65
section .bss

